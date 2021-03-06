open Core
open Async
open Kx

let src = Logs.Src.create "kx.async"

module Log = (val Logs.src_log src : Logs.LOG)

module Log_async = (val Logs_async.src_log src : Logs_async.LOG)

type msg =
  | K : { big_endian : bool; typ : Kx.msgtyp; w : 'a w; msg : 'a } -> msg

let create ?(typ = Async) ?(big_endian = Sys.big_endian) w msg =
  K { big_endian; typ; w; msg }

let pp_serialized ppf (K { big_endian; typ; w; msg }) =
  let serialized = construct_bigstring ~big_endian ~typ w msg in
  Format.fprintf ppf "0x%a" Hex.pp (Hex.of_bigstring serialized)

let handle_chunk wbuf pos nb_to_read =
  if pos < 0 then invalid_arg "pos";
  if nb_to_read < 1 || nb_to_read > Bigstring.length wbuf - pos then
    invalid_arg "nb_to_read";
  let nb_to_read = ref nb_to_read in
  let wpos = ref pos in
  let inner buf ~pos ~len =
    let will_read = min len !nb_to_read in
    Bigstring.blito ~src:buf ~src_pos:pos ~src_len:will_read ~dst:wbuf
      ~dst_pos:!wpos ();
    if will_read = !nb_to_read then return (`Stop_consumed ((), will_read))
    else (
      wpos := !wpos + will_read;
      nb_to_read := !nb_to_read - will_read;
      return (`Consumed (will_read, `Need !nb_to_read)) )
  in
  inner

let parse_compressed ~big_endian ~msglen w r =
  let msglen = Int32.to_int_exn msglen in
  let module E = (val getmod big_endian : ENDIAN) in
  Angstrom_async.parse E.any_int32 r >>=? fun uncompLen ->
  let uncompLen = Int32.to_int_exn uncompLen in
  let compMsg = Bigstring.create msglen in
  Reader.read_one_chunk_at_a_time r
    ~handle_chunk:(handle_chunk compMsg 12 (msglen - 12))
  >>| fun _ ->
  let uncompMsg = Bigstringaf.create uncompLen in
  uncompress uncompMsg compMsg;
  Angstrom.parse_bigstring (destruct ~big_endian w)
    (Bigstringaf.sub uncompMsg ~off:8 ~len:(uncompLen - 8))

let write ?(big_endian = Sys.big_endian) ?(typ = Async) writer w x =
  let module FE =
  (val Faraday.(if big_endian then (module BE : FE) else (module LE : FE)))
  in
  let buf = Faraday.create 4096 in
  construct (module FE) buf w x;
  Faraday.close buf;
  let len = Faraday.pending_bytes buf in
  Writer.write_char writer (if big_endian then '\x00' else '\x01');
  Writer.write_char writer (char_of_msgtyp typ);
  Writer.write_char writer '\x00';
  Writer.write_char writer '\x00';
  let lenbuf = Bigstring.create 4 in
  Bigstring.(if big_endian then unsafe_set_int32_be else unsafe_set_int32_le)
    lenbuf ~pos:0 (8 + len);
  Writer.write_bigstring writer lenbuf;
  Faraday_async.serialize buf
    ~yield:(fun _ -> Scheduler.yield ())
    ~writev:(fun iovecs ->
      let q = Queue.create () in
      let len =
        List.fold_left iovecs ~init:0 ~f:(fun a iovec ->
            Queue.enqueue q (Obj.magic iovec);
            a + iovec.len)
      in
      Writer.schedule_iovecs writer q;
      Writer.flushed writer >>| fun () -> `Ok len)

module T = struct
  module Address = Uri_sexp

  type t = { r : 'a. 'a w option -> 'a Deferred.t; w : msg Pipe.Writer.t }

  let close { w; _ } =
    Pipe.close w;
    Deferred.unit

  let is_closed { w; _ } = Pipe.is_closed w

  let close_finished { w; _ } = Pipe.closed w
end

include T

let empty =
  {
    r = (fun _ -> failwith "disconnected");
    w =
      Pipe.create_writer (fun r ->
          Pipe.close_read r;
          Deferred.unit);
  }

let read r w =
  Reader.peek r ~len:8 >>= fun _ ->
  Angstrom_async.parse hdr r
  >>=? fun ({ big_endian; typ = _; compressed; len } as hdr) ->
  Log_async.debug (fun m -> m "%a" Kx.pp_print_hdr hdr) >>= fun () ->
  match compressed with
  | false -> Angstrom_async.parse (destruct ~big_endian w) r
  | true -> parse_compressed ~big_endian ~msglen:len w r

let handshake url r w =
  Writer.write w
    (Printf.sprintf "%s:%s\x03\x00"
       (Option.value ~default:"" (Uri.user url))
       (Option.value ~default:"" (Uri.password url)));
  Reader.read_char r >>= function
  | `Eof -> failwith "EOF"
  | `Ok '\x03' -> Deferred.unit
  | `Ok c -> failwithf "Invalid handshake %C" c ()

let process ?(monitor = Monitor.current ()) url r w =
  handshake url r w >>= fun () ->
  Monitor.detach_and_iter_errors (Writer.monitor w) ~f:(fun exn ->
      Writer.close w >>> fun () -> Monitor.send_exn monitor exn);
  let protected_read = function
    | None -> Reader.close r >>= fun () -> raise Exit
    | Some wit -> (
        if Reader.is_closed r then
          invalid_arg "Kx_async: Cannot read from closed reader";
        read r wit >>= function
        | Error msg -> Writer.close w >>= fun () -> failwith msg
        | Ok v -> return v )
  in
  let client_write =
    let send_client_msgs wr rd =
      Pipe.iter ~continue_on_error:false rd
        ~f:(fun (K { big_endian; typ; w; msg }) ->
          write ~typ ~big_endian wr w msg >>= fun () ->
          Log_async.debug (fun m ->
              m "%b %a @[%a@]" big_endian Sexp.pp (Kx.sexp_of_msgtyp typ)
                (Kx.pp w) msg))
    in
    Pipe.create_writer (fun r ->
        Deferred.any_unit [ send_client_msgs w r; Writer.close_started w ])
  in
  don't_wait_for (Pipe.closed client_write >>= fun () -> Writer.close w);
  return { r = protected_read; w = client_write }

let with_connection ~url ~f =
  Async_uri.with_connection ~url ~f:(fun { r; w; _ } ->
      process url r w >>= fun a ->
      Monitor.protect
        (fun () -> f a)
        ~finally:(fun () ->
          Pipe.close a.w;
          Monitor.try_with ~extract_exn:true (fun () -> a.r None) >>= function
          | Ok _ -> assert false
          | Error Exit -> Deferred.unit
          | Error a -> raise a))

module Persistent = struct
  include Persistent_connection_kernel.Make (T)

  let with_current_connection_exn c ~f =
    match current_connection c with
    | None -> failwith "No connection to KDB+ available"
    | Some c -> f c

  let create' ?monitor =
    create ~connect:(fun addr ->
        Monitor.try_with_or_error (fun () ->
            Async_uri.connect addr >>= fun { r; w; _ } ->
            process ?monitor addr r w))
end

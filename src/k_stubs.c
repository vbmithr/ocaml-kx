/*---------------------------------------------------------------------------
   Copyright (c) 2018 Vincent Bernardoff. All rights reserved.
   Distributed under the ISC license, see terms at the end of the file.
   --------------------------------------------------------------------------*/

#include <string.h>
#include <errno.h>
#include <caml/mlvalues.h>
#include <caml/alloc.h>
#include <caml/memory.h>
#include <caml/custom.h>
#include <caml/bigarray.h>
#include "k.h"

#define K_val(v) (*((K*) Data_custom_val(v)))

static int compare_K(value a, value b) {
    K aa = K_val(a), bb = K_val(b);
    return (aa == bb ? 0 : (aa < bb ? -1 : 1));
}

static void finalize_K(value k) {
    r0(K_val(k));
}

static struct custom_operations kx_K_ops =
    { .identifier = "kx_K",
      .finalize = finalize_K,
      .compare = compare_K,
      .compare_ext = custom_compare_ext_default,
      .hash = custom_hash_default,
      .serialize = custom_serialize_default,
      .deserialize = custom_deserialize_default };

static value caml_alloc_K (K a) {
    value custom = caml_alloc_custom(&kx_K_ops, sizeof(K), 0, 1);
    K_val(custom) = a;
    return custom;
}

CAMLprim value r0_stub (value k) { r0(K_val(k)); return Val_unit; }
CAMLprim value k_objtyp (value k) { return Val_int(K_val(k)->t); }
CAMLprim value k_objattrs (value k) { return Val_int(K_val(k)->u); }
CAMLprim value k_refcount (value k) { return Val_int(K_val(k)->r); }
CAMLprim value k_length (value k) { return caml_copy_int64(K_val(k)->n); }
CAMLprim value k_g (value k) { return Val_int(K_val(k)->g); }
CAMLprim value k_h (value k) { return Val_int(K_val(k)->h); }
CAMLprim value k_i (value k) {
    CAMLparam1(k);
    CAMLlocal1(ret);
    ret = caml_copy_int32(K_val(k)->i);
    CAMLreturn(ret);
}
CAMLprim value k_j (value k) {
    CAMLparam1(k);
    CAMLlocal1(ret);
    ret = caml_copy_int64(K_val(k)->j);
    CAMLreturn(ret);
}
CAMLprim value k_e (value k) {
    CAMLparam1(k);
    CAMLlocal1(ret);
    ret = caml_copy_double(K_val(k)->e);
    CAMLreturn(ret);
}
CAMLprim value k_f (value k) {
    CAMLparam1(k);
    CAMLlocal1(ret);
    ret = caml_copy_double(K_val(k)->f);
    CAMLreturn(ret);
}
CAMLprim value k_s (value k) {
    CAMLparam1(k);
    CAMLlocal1(ret);
    ret = caml_copy_string(K_val(k)->s);
    CAMLreturn(ret);
}
CAMLprim value k_k (value k) {
    CAMLparam1(k);
    CAMLlocal1(ret);
    ret = caml_alloc_K(K_val(k)->k);
    CAMLreturn(ret);
}

CAMLprim value k_u (value k) {
    CAMLparam1(k);
    CAMLlocal1(ret);
    ret = caml_alloc_initialized_string(16, K_val(k)->G0);
    CAMLreturn(ret);
}

/* List accessors */

CAMLprim value kK_stub (value k, value i) {
    CAMLparam2(k, i);
    CAMLlocal1(ret);
    ret = caml_alloc_K(kK(K_val(k))[Long_val(i)]);
    CAMLreturn(ret);
}

CAMLprim value kK_set_stub (value k, value i, value e) {
    kK(K_val(k))[Long_val(i)] = K_val(e);
    return Val_unit;
}

CAMLprim value kS_stub (value k, value i) {
    CAMLparam2(k, i);
    CAMLlocal1(ret);
    ret = caml_copy_string(kS(K_val(k))[Long_val(i)]);
    CAMLreturn(ret);
}

CAMLprim value kS_set_stub (value k, value i, value s) {
    kS(K_val(k))[Long_val(i)] = ss(String_val(s));
    return Val_unit;
}

CAMLprim value kG_stub (value k) {
    CAMLparam1(k);
    CAMLlocal1(ret);
    K n = r1(K_val(k));
    K_val(k) = n;
    ret = caml_ba_alloc_dims(CAML_BA_UINT8 | CAML_BA_C_LAYOUT,
                             1, kG(K_val(k)), K_val(k)->n);
    CAMLreturn(ret);
}

CAMLprim value kH_stub (value k) {
    CAMLparam1(k);
    CAMLlocal1(ret);
    ret = caml_ba_alloc_dims(CAML_BA_SINT16 | CAML_BA_C_LAYOUT,
                             1, kH(K_val(k)), K_val(k)->n);
    CAMLreturn(ret);
}

CAMLprim value kI_stub (value k) {
    CAMLparam1(k);
    CAMLlocal1(ret);
    ret = caml_ba_alloc_dims(CAML_BA_INT32 | CAML_BA_C_LAYOUT,
                             1, kI(K_val(k)), K_val(k)->n);
    CAMLreturn(ret);
}

CAMLprim value kJ_stub (value k) {
    CAMLparam1(k);
    CAMLlocal1(ret);
    ret = caml_ba_alloc_dims(CAML_BA_INT64 | CAML_BA_C_LAYOUT,
                             1, kJ(K_val(k)), K_val(k)->n);
    CAMLreturn(ret);
}

CAMLprim value kE_stub (value k) {
    CAMLparam1(k);
    CAMLlocal1(ret);
    ret = caml_ba_alloc_dims(CAML_BA_FLOAT32 | CAML_BA_C_LAYOUT,
                             1, kE(K_val(k)), K_val(k)->n);
    CAMLreturn(ret);
}

CAMLprim value kF_stub (value k) {
    CAMLparam1(k);
    CAMLlocal1(ret);
    ret = caml_ba_alloc_dims(CAML_BA_FLOAT64 | CAML_BA_C_LAYOUT,
                             1, kF(K_val(k)), K_val(k)->n);
    CAMLreturn(ret);
}

CAMLprim value kb_stub (value b) {
    CAMLparam1(b);
    CAMLlocal1(k);
    k = caml_alloc_K(kb(Bool_val(b)));
    CAMLreturn(k);
}

CAMLprim value ku_stub (value u) {
    U uu;
    memcpy(uu.g, String_val(u), 16);
    CAMLparam1(u);
    CAMLlocal1(k);
    k = caml_alloc_K(ku(uu));
    CAMLreturn(k);
}

CAMLprim value kc_stub (value i) {
    CAMLparam1(i);
    CAMLlocal1(k);
    k = caml_alloc_K(kc(Int_val(i)));
    CAMLreturn(k);
}
CAMLprim value kg_stub (value i) {
    CAMLparam1(i);
    CAMLlocal1(k);
    k = caml_alloc_K(kg(Int_val(i)));
    CAMLreturn(k);
}
CAMLprim value kh_stub (value i) {
    CAMLparam1(i);
    CAMLlocal1(k);
    k = caml_alloc_K(kh(Int_val(i)));
    CAMLreturn(k);
}
CAMLprim value ki_stub (value i) {
    CAMLparam1(i);
    CAMLlocal1(k);
    k = caml_alloc_K(ki(Int32_val(i)));
    CAMLreturn(k);
}
CAMLprim value kj_stub (value i) {
    CAMLparam1(i);
    CAMLlocal1(k);
    k = caml_alloc_K(kj(Int64_val(i)));
    CAMLreturn(k);
}

CAMLprim value ke_stub (value f) {
    CAMLparam1(f);
    CAMLlocal1(k);
    k = caml_alloc_K(ke(Double_val(f)));
    CAMLreturn(k);
}
CAMLprim value kf_stub (value f) {
    CAMLparam1(f);
    CAMLlocal1(k);
    k = caml_alloc_K(kf(Double_val(f)));
    CAMLreturn(k);
}

CAMLprim value ks_stub (value s) {
    CAMLparam1(s);
    CAMLlocal1(k);
    k = caml_alloc_K(ks(String_val(s)));
    CAMLreturn(k);
}
CAMLprim value kp_stub (value s) {
    CAMLparam1(s);
    CAMLlocal1(k);
    k = caml_alloc_K(kp(String_val(s)));
    CAMLreturn(k);
}
CAMLprim value kpn_stub (value s, value n) {
    CAMLparam1(s);
    CAMLlocal1(k);
    k = caml_alloc_K(kpn(String_val(s), Int_val(n)));
    CAMLreturn(k);
}

CAMLprim value ktimestamp_stub (value j) {
    CAMLparam1(j);
    CAMLlocal1(k);
    k = caml_alloc_K(ktj(-KP, Int64_val(j)));
    CAMLreturn(k);
}
CAMLprim value kt_stub (value i) {
    CAMLparam1(i);
    CAMLlocal1(k);
    k = caml_alloc_K(kt(Int32_val(i)));
    CAMLreturn(k);
}
CAMLprim value kd_stub (value i) {
    CAMLparam1(i);
    CAMLlocal1(k);
    k = caml_alloc_K(kd(Int32_val(i)));
    CAMLreturn(k);
}

CAMLprim value kmonth_stub(value i) {
    CAMLparam1(i);
    CAMLlocal1(kk);
    K k = ka(-KM);
    k->i = Int32_val(i);
    kk = caml_alloc_K(k);
    CAMLreturn(kk);
}
CAMLprim value kminute_stub(value i) {
    CAMLparam1(i);
    CAMLlocal1(kk);
    K k = ka(-KU);
    k->i = Int32_val(i);
    kk = caml_alloc_K(k);
    CAMLreturn(kk);
}
CAMLprim value ksecond_stub(value i) {
    CAMLparam1(i);
    CAMLlocal1(kk);
    K k = ka(-KV);
    k->i = Int32_val(i);
    kk = caml_alloc_K(k);
    CAMLreturn(kk);
}

CAMLprim value ktimespan_stub (value j) {
    CAMLparam1(j);
    CAMLlocal1(k);
    k = caml_alloc_K(ktj(-KN, Int64_val(j)));
    CAMLreturn(k);
}

CAMLprim value kz_stub (value f) {
    CAMLparam1(f);
    CAMLlocal1(k);
    k = caml_alloc_K(kz(Double_val(f)));
    CAMLreturn(k);
}

CAMLprim value ktn_stub (value t, value len) {
    CAMLparam2(t, len);
    CAMLlocal1(k);
    k = caml_alloc_K(ktn(Int_val(t), Int64_val(len)));
    CAMLreturn(k);
}

CAMLprim value xD_stub (value keys, value values) {
    CAMLparam2(keys, values);
    CAMLlocal1(k);
    k = caml_alloc_K(xD(K_val(keys), K_val(values)));
    CAMLreturn(k);
}

CAMLprim value xT_stub (value dict) {
    CAMLparam1(dict);
    CAMLlocal1(k);
    k = caml_alloc_K(xT(K_val(dict)));
    CAMLreturn(k);
}

CAMLprim value ktd_stub (value kt) {
    CAMLparam1(kt);
    CAMLlocal1(k);
    k = caml_alloc_K(ktd(K_val(kt)));
    CAMLreturn(k);
}

CAMLprim value ja_int_stub (value k, value i) {
    int ii = Int_val(i);
    K ret = ja(Data_custom_val(k), &ii);
    K_val(k) = ret;
    return Val_unit;
}

CAMLprim value ja_long_stub (value k, value j) {
    int jj = Long_val(j);
    K ret = ja(Data_custom_val(k), &jj);
    K_val(k) = ret;
    return Val_unit;
}

CAMLprim value ja_int32_stub (value k, value i) {
    int ii = Int32_val(i);
    K ret = ja(Data_custom_val(k), &ii);
    K_val(k) = ret;
    return Val_unit;
}

CAMLprim value ja_int64_stub (value k, value j) {
    long jj = Int64_val(j);
    K ret = ja(Data_custom_val(k), &jj);
    K_val(k) = ret;
    return Val_unit;
}

CAMLprim value ja_double_stub (value k, value f) {
    double ff = Double_val(f);
    K ret = ja(Data_custom_val(k), &ff);
    K_val(k) = ret;
    return Val_unit;
}

CAMLprim value ja_bool_stub (value k, value b) {
    int bb = Bool_val(b);
    K ret = ja(Data_custom_val(k), &bb);
    K_val(k) = ret;
    return Val_unit;
}

CAMLprim value ja_uuid_stub(value k, value u) {
    K ret = ja(Data_custom_val(k), String_val(u));
    K_val(k) = ret;
    return Val_unit;
}

CAMLprim value js_stub (value k, value s) {
    K ret = js(Data_custom_val(k), ss(String_val(s)));
    K_val(k) = ret;
    return Val_unit;
}

CAMLprim value jk_stub (value k, value v) {
    K ret = jk(Data_custom_val(k), K_val(v));
    K_val(k) = ret;
    return Val_unit;
}

CAMLprim value jv_stub (value k, value v) {
    K ret = jv(Data_custom_val(k), K_val(v));
    K_val(k) = ret;
    return Val_unit;
}

CAMLprim value d9_stub (value k) {
    CAMLparam1(k);
    CAMLlocal2(ret, errmsg);
    K x = ee(d9(K_val(k)));
    if (xt == -128) {
        ret = caml_alloc(1, 1);
        errmsg = caml_copy_string(x->s?x->s:"");
    }
    else {
        ret = caml_alloc(1, 0);
        Store_field(ret, 0, caml_alloc_K(x));
    }
    CAMLreturn(ret);
}

CAMLprim value b9_stub (value mode, value k) {
    CAMLparam2(mode, k);
    CAMLlocal2(ret, errmsg);
    K x = ee(b9(Int_val(mode), K_val(k)));
    if (xt == -128) {
        ret = caml_alloc(1, 1);
        errmsg = caml_copy_string(x->s?x->s:"");
    }
    else {
        ret = caml_alloc(1, 0);
        Store_field(ret, 0, caml_alloc_K(x));
    }
    CAMLreturn(ret);
}

CAMLprim value khp_stub (value host, value port) {
    return Val_int(khp(String_val(host), Int_val(port)));
}

CAMLprim value khpu_stub (value host, value port, value username) {
    return Val_int(khpu(String_val(host), Int_val(port), String_val(username)));
}

CAMLprim value khpun_stub(value host, value port,
                          value username, value timeout) {
    return Val_int(khpun(String_val(host), Int_val(port),
                         String_val(username), Int_val(timeout)));
}
CAMLprim value khpunc_stub(value host, value port,
                           value username, value timeout, value capability) {
    return Val_int(khpunc(String_val(host),
                         Int_val(port),
                         String_val(username),
                         Int_val(timeout),
                         Int_val(capability)));
}

CAMLprim value kclose_stub(value fd) {
    kclose(Int_val(fd));
    return Val_unit;
}

CAMLprim value kread_stub(value fd) {
    CAMLparam1(fd);
    CAMLlocal1(kk);
    K r = k(Int_val(fd), (S)NULL);
    if (!r) {
        kk = caml_alloc_K(krr("Connection closed"));
        r0(r);
    }
    else {
        kk = caml_alloc_K(r);
        r0(r);
    }
    CAMLreturn(kk);
}

CAMLprim value k0_stub(value fd, value msg) {
    K r = k(-Int_val(fd), String_val(msg), (K)NULL);
    return Val_bool(r->g);
}
CAMLprim value k1_stub(value fd, value msg, value a) {
    K r = k(-Int_val(fd), String_val(msg), K_val(a), (K)NULL);
    return Val_bool(r->g);
}
CAMLprim value k2_stub(value fd, value msg, value a, value b) {
    K r = k(-Int_val(fd), String_val(msg), K_val(a), K_val(b), (K)NULL);
    return Val_bool(r->g);
}
CAMLprim value k3_stub(value fd, value msg, value a, value b, value c) {
    K r = k(-Int_val(fd), String_val(msg), K_val(a), K_val(b), K_val(c), (K)NULL);
    return Val_bool(r->g);
}
CAMLprim value kn_stub(value fd, value msg, value a) {
    K r;
    switch (Wosize_val(a)) {
    case 0:
        r = k(-Int_val(fd), String_val(msg), (K)NULL);
        break;
    case 1:
        r = k(-Int_val(fd), String_val(msg),
              K_val(Field(a, 0)), (K)NULL);
        break;
    case 2:
        r = k(-Int_val(fd), String_val(msg),
              K_val(Field(a, 0)), K_val(Field(a, 1)), (K)NULL);
        break;
    case 3:
        r = k(-Int_val(fd), String_val(msg),
              K_val(Field(a, 0)), K_val(Field(a, 1)),
              K_val(Field(a, 2)), (K)NULL);
        break;
    case 4:
        r = k(-Int_val(fd), String_val(msg),
              K_val(Field(a, 0)), K_val(Field(a, 1)),
              K_val(Field(a, 2)), K_val(Field(a, 3)), (K)NULL);
        break;
    case 5:
        r = k(-Int_val(fd), String_val(msg),
              K_val(Field(a, 0)), K_val(Field(a, 1)),
              K_val(Field(a, 2)), K_val(Field(a, 3)),
              K_val(Field(a, 4)), (K)NULL);
        break;
    case 6:
        r = k(-Int_val(fd), String_val(msg),
              K_val(Field(a, 0)), K_val(Field(a, 1)),
              K_val(Field(a, 2)), K_val(Field(a, 3)),
              K_val(Field(a, 4)), K_val(Field(a, 5)), (K)NULL);
        break;
    }
    return Val_bool(r->g);
}

CAMLprim value ymd_stub(value year, value month, value day) {
    return Val_int(ymd(Int_val(year), Int_val(month), Int_val(day)));
}
CAMLprim value dj_stub(value i) {
    return Val_int(dj(Int_val(i)));
}


/*---------------------------------------------------------------------------
   Copyright (c) 2018 Vincent Bernardoff

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
   WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
   MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
   ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
   ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
   OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
  ---------------------------------------------------------------------------*/

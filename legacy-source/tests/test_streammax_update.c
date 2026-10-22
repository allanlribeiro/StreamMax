#include "streammax_update.h"
#include <assert.h>
#include <string.h>
int main(void) {
  unsigned long v[3];
  assert(sm_update_tripla("1.0.1", v) && v[0]==1 && v[1]==0 && v[2]==1);
  assert(!sm_update_tripla("1.0.1-beta", v));
  assert(!sm_update_tripla("1.0", v));
  assert(!sm_update_tripla("1.0.1.3", v));
  assert(!sm_update_tripla("-1.0.1", v));
  assert(!sm_update_tripla("99999999999.0.0", v));
  assert(sm_update_tag_valida("v1.0.2"));
  assert(!sm_update_tag_valida("1.0.2"));
  assert(!sm_update_tag_valida("v1.0.2-rc1"));
  assert(!sm_update_nova_versao("v1.0.0"));
  assert(!sm_update_nova_versao("v1.0.1"));
  assert(!sm_update_nova_versao("v1.0.2"));
  assert(!sm_update_nova_versao("v1.0.3"));
  assert(!sm_update_nova_versao("v1.0.4"));
  assert(!sm_update_nova_versao("v1.0.5"));
  assert(!sm_update_nova_versao("v1.0.6"));
  assert(!sm_update_nova_versao("v1.0.7"));
  assert(!sm_update_nova_versao("v1.0.8"));
  assert(!sm_update_nova_versao("v1.0.9"));
  assert(!sm_update_nova_versao("v1.0.10"));
  assert(!sm_update_nova_versao("v1.0.11"));
  assert(sm_update_nova_versao("v1.0.12"));
  assert(sm_update_nova_versao("v1.1.0"));
  assert(sm_update_nova_versao("v2.0.0"));
  assert(!sm_update_nova_versao("v0.9.9"));
  assert(!sm_update_nova_versao("v1.0.2-beta"));
  assert(sm_update_url_apk_valida("v1.0.2",
      "https://github.com/allanlribeiro/StreamMax/releases/download/v1.0.2/StreamMax-1.0.2-android.apk"));
  assert(!sm_update_url_apk_valida("v1.0.2",
      "https://github.com/iqui27/nuvio-native-legacy/releases/download/v1.0.2/StreamMax-1.0.2-android.apk"));
  assert(!sm_update_url_apk_valida("v1.0.2",
      "https://github.com/allanlribeiro/StreamMax/releases/download/v1.0.2/StreamMax-1.0.3-android.apk"));
  assert(!sm_update_url_apk_valida("v1.0.2",
      "https://github.com/allanlribeiro/StreamMax/releases/download/v1.0.2/Nuvio-1.0.2-android.apk"));
  assert(!sm_update_url_apk_valida("v1.0.2",
      "https://evil.example/app.apk"));
  return 0;
}

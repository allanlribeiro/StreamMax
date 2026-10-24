#!/usr/bin/env bash
set -euo pipefail
sdk="$1"
export TERM="${TERM:-xterm}"
export PATH="$sdk/tools/ide/bin:$sdk/tools:$sdk/tools/emulator/bin:$PATH"
mkdir -p output/tizen-emulador "$sdk/data"
printf 'TIZEN_SDK_INSTALLED_PATH=%s\nTIZEN_SDK_DATA_PATH=%s\nJDK_PATH=%s\n' "$sdk" "$sdk/data" "$sdk/jdk" > "$sdk/sdk.info"
test -c /dev/kvm
em-cli create -n StreamMaxTV -p tv-samsung-10.0-x86_64 -t HD1080-TV -w yes -g no -r 1024
vm="$sdk/data/emulator/vms/StreamMaxTV"
if [ ! -s "$vm/swap-StreamMaxTV.img" ]; then
  truncate -s 512M "$vm/swap-StreamMaxTV.img"
  chmod 600 "$vm/swap-StreamMaxTV.img"
  mkswap "$vm/swap-StreamMaxTV.img"
fi
em-cli launch -n StreamMaxTV > output/tizen-emulador/boot.txt 2>&1
sdb start-server
serial=''
for attempt in $(seq 1 120); do
 serial="$(sdb devices | awk 'NR>1 && $2=="device" {print $1;exit}')"
 if [ -n "$serial" ]; then break; fi
 sleep 2
done
if [ -z "$serial" ]; then
 cp -R "$sdk/data/emulator/vms/StreamMaxTV/logs" output/tizen-emulador/ || true
 echo 'Emulador não se conectou ao SDB'; exit 1
fi
sdb -s "$serial" capability > output/tizen-emulador/capability.txt
mkdir -p "$RUNNER_TEMP/sm-cert" "$RUNNER_TEMP/sm-widget"
tizen certificate -a StreamMaxTest -p StreamMaxEmulator2026 -n StreamMax -- "$RUNNER_TEMP/sm-cert"
author="$(find "$RUNNER_TEMP/sm-cert" -name '*.p12' -print -quit)"
"$sdk/jdk/bin/keytool" -list -storetype PKCS12 -keystore "$author" -storepass StreamMaxEmulator2026 > output/tizen-emulador/certificado-teste.txt
unzip -q legacy-source/StreamMax-1.0.17-tizen.wgt -d "$RUNNER_TEMP/sm-widget"
mkdir -p "$RUNNER_TEMP/sm-sign-classes"
"$sdk/jdk/bin/javac" -cp "$sdk/tools/ide/lib-ncli/*" -d "$RUNNER_TEMP/sm-sign-classes" tests/ports/SignEmulatorWidget.java
"$sdk/jdk/bin/java" -Dlog4j.configurationFile="$PWD/tests/ports/tizen-log4j.xml" -cp "$RUNNER_TEMP/sm-sign-classes:$sdk/tools/ide/lib-ncli/*" SignEmulatorWidget "$sdk" "$author" "$RUNNER_TEMP/sm-widget" > output/tizen-emulador/assinatura.txt 2>&1
pkg="$RUNNER_TEMP/StreamMax-emulador-assinado.wgt"
(cd "$RUNNER_TEMP/sm-widget" && zip -q -r -X "$pkg" . -x .manifest.tmp .delta.lst)
tizen install -n "$pkg" -s "$serial" | tee output/tizen-emulador/instalacao.txt
sdb -s "$serial" shell 'app_launcher -w -s StreamMax1.StreamMax' > output/tizen-emulador/debug.txt
port="$(grep -ioE 'port[: ]+[0-9]+' output/tizen-emulador/debug.txt | grep -oE '[0-9]+' | head -1)"
test -n "$port"
sdb -s "$serial" forward tcp:9222 "tcp:$port"
NODE_PATH="$PWD/desktop/node_modules" node tests/ports/tizen-cdp.cjs

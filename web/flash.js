// MacroDesk firmware flasher.
//
// The prebuilt image is a merged flash image written at 0x0, so the bootloader
// header it already carries is what decides flash mode, frequency and size --
// hence "keep" for all three. Rewriting them here would contradict the build.
import { ESPLoader, Transport } from "https://cdn.jsdelivr.net/npm/esptool-js@0.6.1/bundle.js";

const PREBUILT = "../firmware/prebuilt/MacroDesk-esp32s3-touch-lcd-7.bin";
const FLASH_ADDRESS = 0x0;
const FLASH_BAUD = 921600;

const $ = selector => document.querySelector(selector);
const elements = {
  support: $("#support"), log: $("#log"), clearLog: $("#clearLog"), toast: $("#toast"),
  flashButton: $("#flashButton"), eraseButton: $("#eraseButton"), eraseFirst: $("#eraseFirst"),
  pickFile: $("#pickFile"), binFile: $("#binFile"), fileState: $("#fileState"), prebuiltState: $("#prebuiltState"),
  progressBox: $("#progressBox"), progressFill: $("#progressFill"),
  progressLabel: $("#progressLabel"), progressPercent: $("#progressPercent")
};

let chosenFile = null;
let busy = false;
let toastTimer = null;

const supported = "serial" in navigator;
if (!supported) {
  elements.support.hidden = false;
  elements.support.textContent = location.protocol === "file:"
    ? "This page is open from a file:// path, where browsers block USB access. Serve the repository over http://localhost or https:// and reload."
    : "This browser has no Web Serial support. Use Chrome or Edge on Windows, macOS or Linux — Safari and mobile browsers cannot flash.";
  elements.flashButton.disabled = true;
  elements.eraseButton.disabled = true;
}

function log(line) {
  const stamp = new Date().toLocaleTimeString();
  elements.log.textContent = elements.log.textContent === "Waiting for you to start."
    ? `[${stamp}] ${line}`
    : `${elements.log.textContent}\n[${stamp}] ${line}`;
  elements.log.scrollTop = elements.log.scrollHeight;
}

function showToast(message) {
  elements.toast.textContent = message;
  elements.toast.classList.add("show");
  clearTimeout(toastTimer);
  toastTimer = setTimeout(() => elements.toast.classList.remove("show"), 2600);
}

function setProgress(label, fraction) {
  elements.progressBox.hidden = false;
  const percent = Math.max(0, Math.min(100, Math.round(fraction * 100)));
  elements.progressLabel.textContent = label;
  elements.progressPercent.textContent = `${percent}%`;
  elements.progressFill.style.width = `${percent}%`;
}

function setBusy(value) {
  busy = value;
  elements.flashButton.disabled = value || !supported;
  elements.eraseButton.disabled = value || !supported;
  elements.flashButton.textContent = value ? "Working…" : "Connect and flash";
}

// esptool-js writes its own progress and chip details through this interface.
const terminal = {
  clean() { elements.log.textContent = "Waiting for you to start."; },
  writeLine(data) { log(data); },
  write(data) { if (String(data).trim()) log(String(data).trim()); }
};

const selectedSource = () => document.querySelector('input[name="source"]:checked').value;

elements.pickFile.addEventListener("click", () => elements.binFile.click());
elements.binFile.addEventListener("change", event => {
  const file = event.target.files[0];
  event.target.value = "";
  if (!file) return;
  chosenFile = file;
  elements.fileState.textContent = `${file.name} · ${(file.size / 1024 / 1024).toFixed(2)} MB`;
  document.querySelector('input[name="source"][value="file"]').checked = true;
  log(`Selected ${file.name} (${file.size.toLocaleString()} bytes).`);
});

elements.clearLog.addEventListener("click", () => { elements.log.textContent = "Waiting for you to start."; });

async function loadFirmware() {
  if (selectedSource() === "file") {
    if (!chosenFile) throw new Error("Choose a .bin file first, or switch back to the bundled build.");
    return { name: chosenFile.name, data: new Uint8Array(await chosenFile.arrayBuffer()) };
  }
  const response = await fetch(PREBUILT);
  if (!response.ok) throw new Error(`Could not read the bundled firmware (HTTP ${response.status}). Serve the repository root so ${PREBUILT} resolves.`);
  return { name: PREBUILT.split("/").pop(), data: new Uint8Array(await response.arrayBuffer()) };
}

// An ESP32 image starts with magic byte 0xE9. Catching a wrong file here is far
// kinder than letting the board end up in a boot loop.
function checkImage(data) {
  if (data.length < 1024) throw new Error("That file is too small to be a firmware image.");
  if (data[0] !== 0xE9) throw new Error("That file does not start with the ESP32 image magic byte (0xE9). It is probably not a firmware .bin.");
}

async function withLoader(action, label) {
  if (busy) return;
  let transport = null;
  setBusy(true);
  try {
    const port = await navigator.serial.requestPort();
    transport = new Transport(port, true);
    const loader = new ESPLoader({ transport, baudrate: FLASH_BAUD, terminal });
    setProgress("Connecting to the board…", 0);
    const chip = await loader.main();
    log(`Connected to ${chip}.`);
    await action(loader);
    setProgress("Done", 1);
    log("Resetting the board.");
    await loader.after("hard_reset");
    showToast(`${label} finished`);
  } catch (error) {
    const message = error && error.message ? error.message : String(error);
    if (/No port selected|cancelled/i.test(message)) {
      log("Cancelled — no port was selected.");
    } else {
      log(`Failed: ${message}`);
      showToast(`${label} failed — see the log`);
    }
    setProgress("Stopped", 0);
  } finally {
    if (transport) { try { await transport.disconnect(); } catch (_) { /* already gone */ } }
    setBusy(false);
  }
}

elements.flashButton.addEventListener("click", () => withLoader(async loader => {
  const firmware = await loadFirmware();
  checkImage(firmware.data);
  log(`Writing ${firmware.name} — ${firmware.data.length.toLocaleString()} bytes at 0x${FLASH_ADDRESS.toString(16)}.`);
  await loader.writeFlash({
    fileArray: [{ data: firmware.data, address: FLASH_ADDRESS }],
    flashMode: "keep",
    flashFreq: "keep",
    flashSize: "keep",
    eraseAll: elements.eraseFirst.checked,
    compress: true,
    reportProgress: (_index, written, total) => setProgress("Writing firmware…", written / total)
  });
}, "Flashing"));

elements.eraseButton.addEventListener("click", () => withLoader(async loader => {
  setProgress("Erasing flash…", 0);
  log("Erasing the whole flash. This takes a while on a 16 MB board.");
  await loader.eraseFlash();
}, "Erase"));

fetch(PREBUILT, { method: "HEAD" })
  .then(response => {
    // Some static servers omit Content-Length on HEAD, so only show a size when we get one.
    const bytes = Number(response.headers.get("content-length") || 0);
    elements.prebuiltState.textContent = response.ok
      ? `MacroDesk-esp32s3-touch-lcd-7.bin${bytes ? ` · ${(bytes / 1024 / 1024).toFixed(2)} MB` : ""}`
      : "Not reachable from here — serve the repository root, or pick your own .bin";
  })
  .catch(() => { elements.prebuiltState.textContent = "Not reachable from here — pick your own .bin instead"; });

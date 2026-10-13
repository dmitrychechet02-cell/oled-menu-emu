// Веб-эмулятор: загружает C-ядро (WebAssembly) и рисует «RAM дисплея» на canvas.
// Логики меню здесь нет — всё в src/, здесь только ввод/вывод.

const WIDTH = 128;
const HEIGHT = 64;
const SCALE = 5;
const TICK_MS = 100;

const KEYS = { none: 0, up: 1, down: 2, ok: 3, back: 4 };
const SCREENS = ["menu", "readings", "symbols", "about"];
const COLORS = {
  white: [235, 244, 255],
  blue: [80, 190, 255],
  yellow: [255, 214, 64],
  green: [120, 255, 150],
};

const canvas = document.getElementById("screen");
const ctx = canvas.getContext("2d");
const colorSelect = document.getElementById("color");
const errorBox = document.getElementById("error");
const out = {
  title: document.querySelector('[data-testid="state-title"]'),
  nav: document.querySelector('[data-testid="state-nav"]'),
  contrast: document.querySelector('[data-testid="state-contrast"]'),
  power: document.querySelector('[data-testid="state-power"]'),
};

let wasm = null;
let color = COLORS.white;

function readCString(ptr) {
  const mem = new Uint8Array(wasm.memory.buffer);
  let end = ptr;
  while (mem[end] !== 0) end++;
  return new TextDecoder().decode(mem.subarray(ptr, end));
}

function displayRam() {
  return new Uint8Array(wasm.memory.buffer, wasm.emu_display_ram(), (WIDTH * HEIGHT) / 8);
}

function pixel(x, y) {
  const ram = displayRam();
  return (ram[(y >> 3) * WIDTH + x] >> (y & 7)) & 1;
}

function state() {
  return {
    title: readCString(wasm.emu_title()),
    screen: SCREENS[wasm.emu_screen_id()] ?? "unknown",
    depth: wasm.emu_menu_depth(),
    selected: wasm.emu_selected(),
    editing: wasm.emu_editing() === 1,
    contrast: wasm.emu_contrast(),
    inverted: wasm.emu_inverted() === 1,
    powered: wasm.emu_powered() === 1,
    brightness: wasm.emu_brightness(),
    lang: wasm.emu_lang() === 0 ? "ru" : "en",
    flushes: wasm.emu_flush_count(),
  };
}

function draw() {
  const s = state();
  ctx.fillStyle = "#050607";
  ctx.fillRect(0, 0, canvas.width, canvas.height);

  if (s.powered) {
    // Контраст SSD1309 0..255 -> яркость пикселя.
    const alpha = 0.3 + (0.7 * s.contrast) / 255;
    const [r, g, b] = color;
    ctx.fillStyle = `rgba(${r}, ${g}, ${b}, ${alpha.toFixed(3)})`;
    const ram = displayRam();
    // Каждый бит framebuffer задаёт пиксель, который рисуем на canvas с масштабом SCALE.
    for (let y = 0; y < HEIGHT; y++) {
      for (let x = 0; x < WIDTH; x++) {
        let on = (ram[(y >> 3) * WIDTH + x] >> (y & 7)) & 1;
        if (s.inverted) on ^= 1;
        if (on) ctx.fillRect(x * SCALE, y * SCALE, SCALE - 1, SCALE - 1);
      }
    }
  }

  out.title.textContent = s.title;
  out.nav.textContent = `${s.depth} / ${s.selected}${s.editing ? " (правка)" : ""}`;
  out.contrast.textContent = `${s.contrast}${s.inverted ? ", инверсия" : ""}`;
  out.power.textContent = s.powered ? "включён" : "спит";
}

function press(name) {
  const code = KEYS[name];
  if (code === undefined) throw new Error(`Неизвестная кнопка: ${name}`);
  wasm.emu_key(code);
  draw();
  const btn = document.querySelector(`[data-key="${name}"]`);
  if (btn) {
    btn.classList.add("pressed");
    setTimeout(() => btn.classList.remove("pressed"), 90);
  }
}

function tick(ms) {
  wasm.emu_tick(ms);
  draw();
}

function bindInput() {
  for (const btn of document.querySelectorAll("[data-key]")) {
    btn.addEventListener("click", () => press(btn.dataset.key));
  }
  const map = {
    ArrowUp: "up",
    ArrowDown: "down",
    Enter: "ok",
    ArrowRight: "ok",
    Escape: "back",
    Backspace: "back",
    ArrowLeft: "back",
  };
  window.addEventListener("keydown", (e) => {
    const name = map[e.key];
    if (!name || e.target instanceof HTMLSelectElement) return;
    e.preventDefault();
    press(name);
  });
  colorSelect.addEventListener("change", () => {
    color = COLORS[colorSelect.value] ?? COLORS.white;
    draw();
  });
}

async function loadWasm() {
  const url = new URL("build/emu.wasm", import.meta.url);
  const resp = await fetch(url);
  if (!resp.ok) throw new Error(`${url.pathname}: HTTP ${resp.status}`);
  const bytes = await resp.arrayBuffer();
  const { instance } = await WebAssembly.instantiate(bytes, {});
  return instance.exports;
}

async function main() {
  try {
    wasm = await loadWasm();
  } catch (err) {
    errorBox.hidden = false;
    errorBox.textContent = `Не удалось загрузить emu.wasm (${err.message}). Соберите: make wasm`;
    console.error(err);
    return;
  }

  wasm.emu_init();
  bindInput();
  draw();

  // ?paused=1 — время не идёт само (детерминированные e2e-тесты).
  const paused = new URLSearchParams(location.search).has("paused");
  if (!paused) setInterval(() => tick(TICK_MS), TICK_MS);

  // API для тестов и отладки из консоли браузера.
  window.emu = { press, tick, state, pixel, ready: true };
  document.body.dataset.ready = "true";
}

main();

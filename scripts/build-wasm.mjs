// На Unix используем Makefile; под Windows — clang или локальный Zig с теми же флагами.
import { existsSync, mkdirSync, readdirSync } from "node:fs";
import { join, resolve } from "node:path";
import { spawnSync } from "node:child_process";

const root = resolve(import.meta.dirname, "..");
function run(command, args, env = process.env) {
  const result = spawnSync(command, args, { cwd: root, env, stdio: "inherit" });
  if (result.error) console.error(result.error.message);
  process.exit(result.status ?? 1);
}

if (process.platform !== "win32") run("make", ["wasm"]);

const zig = process.env.ZIG_EXE ?? join(root, "build/tools/zig-x86_64-windows-0.14.1/zig.exe");
const clangTargets = spawnSync("clang", ["--print-targets"], { encoding: "utf8" });
const hasClang = clangTargets.status === 0 && /^\s*wasm32\b/m.test(clangTargets.stdout);
if (!hasClang && !existsSync(zig)) {
  console.error("Нужен clang с wasm32/lld либо Zig: укажи путь к zig.exe в ZIG_EXE. См. README.");
  process.exit(1);
}

const dirs = ["src/gfx", "src/fonts", "src/menu", "src/app", "src/hal/web"];
const sources = dirs.flatMap((dir) =>
  readdirSync(join(root, dir))
    .filter((name) => name.endsWith(".c"))
    .sort()
    .map((name) => `${dir}/${name}`),
);
mkdirSync(join(root, "web/build"), { recursive: true });
const flags = [
  "-std=c11",
  "-Wall",
  "-Wextra",
  "-Werror",
  "-Wshadow",
  "-Wstrict-prototypes",
  "-Wmissing-prototypes",
  "-Wpedantic",
  "-O2",
  "-ffreestanding",
  "-nostdlib",
  "-Wl,--no-entry",
  "-Wl,--strip-debug",
  "-o",
  "web/build/emu.wasm",
  ...sources,
];
if (hasClang) run("clang", ["--target=wasm32", ...flags]);
console.log("Сборка WebAssembly через Zig (clang), предупреждения считаются ошибками.");
run(zig, ["cc", "-target", "wasm32-freestanding", ...flags], {
  ...process.env,
  ZIG_GLOBAL_CACHE_DIR: join(root, "build/zig-cache-global"),
  ZIG_LOCAL_CACHE_DIR: join(root, "build/zig-cache"),
});

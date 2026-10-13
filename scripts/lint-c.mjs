// Те же C-проверки, что в make lint; Windows использует локальный LLVM-MinGW.
import { existsSync, mkdirSync, readdirSync, writeFileSync } from "node:fs";
import { dirname, join, resolve } from "node:path";
import { spawnSync } from "node:child_process";

const root = resolve(import.meta.dirname, "..");
if (process.platform !== "win32") {
  const result = spawnSync("make", ["lint"], { cwd: root, stdio: "inherit" });
  if (result.error) console.error(result.error.message);
  process.exit(result.status ?? 1);
}
const bin = join(root, "build/tools/llvm-mingw-20261006-ucrt-x86_64/bin");
const clang = process.env.C_CLANG ?? join(bin, "clang.exe");
const formatter = process.env.C_CLANG_FORMAT ?? join(dirname(clang), "clang-format.exe");
if (!existsSync(clang) || !existsSync(formatter)) {
  console.error(
    "Нужны clang и clang-format: укажи полные пути C_CLANG и C_CLANG_FORMAT (см. README).",
  );
  process.exit(1);
}
const env = { ...process.env, PATH: `${dirname(clang)};${process.env.PATH ?? ""}` };
const warn = [
  "-std=c11",
  "-Wall",
  "-Wextra",
  "-Werror",
  "-Wshadow",
  "-Wstrict-prototypes",
  "-Wmissing-prototypes",
  "-Wpedantic",
];
const sources = (dir) =>
  readdirSync(join(root, dir))
    .filter((name) => name.endsWith(".c"))
    .sort()
    .map((name) => `${dir}/${name}`);
const core = ["src/gfx", "src/fonts", "src/menu", "src/app"].flatMap(sources);
function cFiles(dir) {
  return readdirSync(join(root, dir), { withFileTypes: true })
    .flatMap((entry) => {
      const file = `${dir}/${entry.name}`;
      return entry.isDirectory() ? cFiles(file) : /\.[ch]$/.test(entry.name) ? [file] : [];
    })
    .sort();
}
mkdirSync(join(root, "build"), { recursive: true });
mkdirSync(join(root, "web/build"), { recursive: true });
let log = "";
function check(label, command, args, checkEnv = env) {
  console.log(label);
  log += `${label}\n`;
  const result = spawnSync(command, args, { cwd: root, env: checkEnv, encoding: "utf8" });
  const output = `${result.stdout ?? ""}${result.stderr ?? ""}${result.error?.message ?? ""}`;
  log += output;
  if (output) process.stdout.write(output);
  log += `exit: ${result.status ?? 1}\n`;
  writeFileSync(join(root, "build/c-lint.log"), log);
  if (result.status !== 0) process.exit(result.status ?? 1);
}
check("ARM Cortex-M: freestanding, -Werror", clang, [
  "--target=armv7m-none-eabi",
  "-mthumb",
  ...warn,
  "-O2",
  "-ffreestanding",
  "-fsyntax-only",
  ...core,
  ...sources("src/hal/ssd1309"),
]);
// Не добавляем LLVM-MinGW в PATH wasm-сборки: в нём отсутствует wasm backend.
check(
  "WebAssembly: freestanding, без libc, -Werror",
  process.execPath,
  [join(root, "scripts/build-wasm.mjs")],
  process.env,
);
check("clang-format: все .c/.h в src/ и tests/unit/", formatter, [
  "--dry-run",
  "--Werror",
  ...cFiles("src"),
  ...cFiles("tests/unit"),
]);
console.log("C lint: ok (ARM + wasm + формат всех C-файлов)");

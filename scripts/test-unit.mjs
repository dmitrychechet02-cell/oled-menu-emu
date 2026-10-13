// Повторяемый прогон C-тестов: Makefile на Unix, LLVM-MinGW с ASan/UBSan на Windows.
import { existsSync, mkdirSync, readdirSync, writeFileSync } from "node:fs";
import { dirname, join, resolve } from "node:path";
import { spawnSync } from "node:child_process";

const root = resolve(import.meta.dirname, "..");
const filter = process.argv[2];
if (process.argv.length > 3) {
  console.error("Использование: npm test -- [подстрока имени теста]");
  process.exit(1);
}
function run(command, args, env = process.env) {
  const result = spawnSync(command, args, { cwd: root, env, encoding: "utf8" });
  if (result.stdout) process.stdout.write(result.stdout);
  if (result.stderr) process.stderr.write(result.stderr);
  if (result.error) console.error(result.error.message);
  return result;
}
if (process.platform !== "win32") {
  const result = run("make", ["test", ...(filter ? [`T=${filter}`] : [])]);
  process.exit(result.status ?? 1);
}

const compiler =
  process.env.UNIT_CC ?? join(root, "build/tools/llvm-mingw-20261006-ucrt-x86_64/bin/clang.exe");
if (!existsSync(compiler)) {
  console.error(
    "Нужен LLVM-MinGW UCRT с ASan/UBSan. Укажи полный путь к clang.exe в UNIT_CC (см. README).",
  );
  process.exit(1);
}
const env = {
  ...process.env,
  PATH: `${dirname(compiler)};${process.env.PATH ?? ""}`,
};
const dirs = ["src/gfx", "src/fonts", "src/menu", "src/app", "tests/unit"];
const sources = dirs.flatMap((dir) =>
  readdirSync(join(root, dir))
    .filter((name) => name.endsWith(".c"))
    .sort()
    .map((name) => `${dir}/${name}`),
);
sources.push("src/hal/ssd1309/ssd1309.c");
mkdirSync(join(root, "build"), { recursive: true });
const binary = join(root, "build/unit_tests.exe");
const flags = [
  "-std=c11",
  "-Wall",
  "-Wextra",
  "-Werror",
  "-Wshadow",
  "-Wstrict-prototypes",
  "-Wmissing-prototypes",
  "-Wpedantic",
  "-Wno-missing-prototypes",
  "-g",
  "-O0",
  "-fsanitize=address,undefined",
  "-fno-sanitize-recover=all",
  "-o",
  binary,
  ...sources,
];
console.log("C unit-тесты: LLVM-MinGW, -Werror, AddressSanitizer + UBSan.");
const build = run(compiler, flags, env);
if (build.status !== 0) process.exit(build.status ?? 1);
const result = run(binary, filter ? [filter] : [], env);
writeFileSync(join(root, "build/unit-tests.log"), (result.stdout ?? "") + (result.stderr ?? ""));
process.exit(result.status ?? 1);

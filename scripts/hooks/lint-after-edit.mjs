#!/usr/bin/env node
// PostToolUse: после правки файла — форматтер + линтер только для этого файла.
//   C (src/, tests/unit/): clang-format -i, затем компиляция с -Werror
//     (src/ — под wasm32 freestanding: заодно ловит случайный #include libc);
//   JS/CSS/HTML (web/, tests/e2e/, scripts/): prettier --write, затем eslint для JS.
// Ошибки линтера → exit 2: агент видит их и обязан исправить.
import { spawnSync } from "node:child_process";
import { existsSync, readFileSync } from "node:fs";
import { extname, join } from "node:path";
import { editedPaths, isCodex, projectRoot, readHookInput, relPath } from "./lib.mjs";

const WARN = "-std=c11 -Wall -Wextra -Werror -Wshadow -Wstrict-prototypes -Wpedantic".split(" ");

function run(cmd, args, cwd) {
  const r = spawnSync(cmd, args, { cwd, encoding: "utf8", shell: process.platform === "win32" });
  if (r.error && r.error.code === "ENOENT") return { missing: true };
  return { code: r.status ?? 1, out: `${r.stdout ?? ""}${r.stderr ?? ""}`.trim() };
}

function lintC(root, rel, notes, errors) {
  const before = readFileSync(join(root, rel), "utf8");
  const fmt = run("clang-format", ["-i", rel], root);
  if (fmt.missing) notes.push("clang-format не найден — форматирование пропущено");
  else if (readFileSync(join(root, rel), "utf8") !== before)
    notes.push(`clang-format: ${rel} переформатирован`);

  if (!rel.endsWith(".c")) return;
  const args = rel.startsWith("src/")
    ? ["--target=wasm32", "-ffreestanding", "-fsyntax-only", ...WARN, rel]
    : ["-fsyntax-only", ...WARN, rel];
  const cc = run("clang", args, root);
  if (cc.missing) notes.push("clang не найден — проверка компиляции пропущена");
  else if (cc.code !== 0) errors.push(`clang ${rel}:\n${cc.out}`);
  else notes.push(`clang -Werror: ${rel} ок`);
}

function lintWeb(root, rel, notes, errors) {
  const bin = (name) => join(root, "node_modules", ".bin", name);
  if (!existsSync(bin("prettier"))) {
    notes.push("prettier не установлен (npm install) — проверка пропущена");
    return;
  }
  const before = readFileSync(join(root, rel), "utf8");
  run(bin("prettier"), ["--write", "--log-level", "warn", rel], root);
  if (readFileSync(join(root, rel), "utf8") !== before)
    notes.push(`prettier: ${rel} переформатирован`);

  if (![".js", ".mjs"].includes(extname(rel))) return;
  const es = run(bin("eslint"), ["--max-warnings", "0", rel], root);
  if (es.code !== 0) errors.push(`eslint ${rel}:\n${es.out}`);
  else notes.push(`eslint: ${rel} ок`);
}

const input = await readHookInput();
const root = projectRoot(input);
const notes = [];
const errors = [];

for (const p of editedPaths(input)) {
  const rel = relPath(root, p);
  if (rel.startsWith("../") || !existsSync(join(root, rel))) continue;
  const ext = extname(rel);
  if ([".c", ".h"].includes(ext) && (rel.startsWith("src/") || rel.startsWith("tests/unit/"))) {
    lintC(root, rel, notes, errors);
  } else if (
    [".js", ".mjs", ".css", ".html"].includes(ext) &&
    /^(web|tests\/e2e|scripts)\//.test(rel) &&
    !rel.startsWith("web/build/")
  ) {
    lintWeb(root, rel, notes, errors);
  }
}

if (errors.length > 0) {
  process.stderr.write(
    `[lint-after-edit] Линтер нашёл ошибки — исправь их:\n\n${errors.join("\n\n")}\n`,
  );
  process.exit(2);
}

if (notes.length > 0) {
  const msg = `[lint-after-edit] ${notes.join("; ")}`;
  if (isCodex(input)) {
    process.stderr.write(`${msg}\n`);
  } else {
    // Claude Code: systemMessage видит человек, additionalContext — модель.
    process.stdout.write(
      JSON.stringify({
        systemMessage: msg,
        hookSpecificOutput: { hookEventName: "PostToolUse", additionalContext: msg },
      }),
    );
  }
}
process.exit(0);

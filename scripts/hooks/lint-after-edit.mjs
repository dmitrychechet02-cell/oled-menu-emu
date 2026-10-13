#!/usr/bin/env node
// PostToolUse: после правки файла — форматтер + линтер только для этого файла.
//   C (src/, tests/unit/): clang-format -i, затем компиляция с -Werror
//     (src/ — под wasm32 freestanding: заодно ловит случайный #include libc);
//   JS/CSS/HTML (web/, tests/e2e/, scripts/): prettier --write, затем eslint для JS.
// Ошибки передаются JSON-протоколом PostToolUse без потери кода в PowerShell.
import { spawnSync } from "node:child_process";
import { existsSync, readFileSync } from "node:fs";
import { extname, join } from "node:path";
import { editedPaths, projectRoot, readHookInput, relPath } from "./lib.mjs";

const WARN = "-std=c11 -Wall -Wextra -Werror -Wshadow -Wstrict-prototypes -Wpedantic".split(" ");

function run(cmd, args, cwd) {
  const r = spawnSync(cmd, args, { cwd, encoding: "utf8" });
  if (r.error && r.error.code === "ENOENT") return { missing: true };
  return { code: r.status ?? 1, out: `${r.stdout ?? ""}${r.stderr ?? ""}`.trim() };
}

function lintC(root, rel, notes, errors) {
  const before = readFileSync(join(root, rel), "utf8");
  const fmt = run("clang-format", ["-i", rel], root);
  if (fmt.missing) errors.push("clang-format не найден — установи инструмент и повтори проверку");
  else if (fmt.code !== 0) errors.push(`clang-format ${rel}:\n${fmt.out}`);
  else if (readFileSync(join(root, rel), "utf8") !== before)
    notes.push(`clang-format: ${rel} переформатирован`);

  if (!rel.endsWith(".c")) return;
  const args = rel.startsWith("src/")
    ? ["--target=wasm32", "-ffreestanding", "-fsyntax-only", ...WARN, rel]
    : ["-fsyntax-only", ...WARN, rel];
  const cc = run("clang", args, root);
  if (cc.missing) errors.push("clang не найден — установи инструмент и повтори проверку");
  else if (cc.code !== 0) errors.push(`clang ${rel}:\n${cc.out}`);
  else notes.push(`clang -Werror: ${rel} ок`);
}

function lintWeb(root, rel, notes, errors) {
  const prettier = join(root, "node_modules", "prettier", "bin", "prettier.cjs");
  const eslint = join(root, "node_modules", "eslint", "bin", "eslint.js");
  if (!existsSync(prettier)) {
    errors.push("prettier не установлен — установи зависимости и повтори проверку");
    return;
  }
  const before = readFileSync(join(root, rel), "utf8");
  const fmt = run(process.execPath, [prettier, "--write", "--log-level", "warn", rel], root);
  if (fmt.code !== 0) {
    errors.push(`prettier ${rel}:\n${fmt.out}`);
    return;
  }
  if (readFileSync(join(root, rel), "utf8") !== before)
    notes.push(`prettier: ${rel} переформатирован`);

  if (![".js", ".mjs"].includes(extname(rel))) return;
  if (!existsSync(eslint)) {
    errors.push("eslint не установлен — установи зависимости и повтори проверку");
    return;
  }
  const es = run(process.execPath, [eslint, "--max-warnings", "0", rel], root);
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

if (errors.length > 0 || notes.length > 0) {
  const msg = errors.length
    ? `[lint-after-edit] Линтер нашёл ошибки — исправь их:\n\n${errors.join("\n\n")}`
    : `[lint-after-edit] ${notes.join("; ")}`;
  const output = {
    systemMessage: msg,
    hookSpecificOutput: { hookEventName: "PostToolUse", additionalContext: msg },
  };
  if (errors.length) {
    output.decision = "block";
    output.reason = msg;
  }
  // ASCII JSON сохраняет сообщения при перекодировке Windows PowerShell.
  const json = JSON.stringify(output).replace(
    /[\u007f-\uffff]/g,
    (ch) => `\\u${ch.charCodeAt(0).toString(16).padStart(4, "0")}`,
  );
  process.stdout.write(`${json}\n`);
}
process.exit(0);

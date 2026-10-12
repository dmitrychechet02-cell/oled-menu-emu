// Общие функции hooks для Claude Code и Codex.
import { isAbsolute, relative, resolve, sep } from "node:path";

/** Прочитать JSON, который агент передаёт hook-у в stdin. */
export async function readHookInput() {
  const chunks = [];
  for await (const chunk of process.stdin) chunks.push(chunk);
  const raw = Buffer.concat(chunks).toString("utf8").trim();
  return raw ? JSON.parse(raw) : {};
}

/** Корень проекта: переменная Claude Code или cwd из входа (Codex), иначе текущий каталог. */
export function projectRoot(input) {
  return process.env.CLAUDE_PROJECT_DIR || input.cwd || process.cwd();
}

/** Codex присылает turn_id; Claude Code — нет. */
export function isCodex(input) {
  return typeof input.turn_id === "string";
}

/**
 * Пути файлов, которые инструмент собирается изменить.
 * Claude Code: Edit/Write/MultiEdit → tool_input.file_path, NotebookEdit → notebook_path.
 * Codex: apply_patch → заголовки «*** Update File: path» в tool_input.command.
 */
export function editedPaths(input) {
  const ti = input.tool_input ?? {};
  const paths = [];
  if (typeof ti.file_path === "string") paths.push(ti.file_path);
  if (typeof ti.notebook_path === "string") paths.push(ti.notebook_path);
  const patch = typeof ti.command === "string" ? ti.command : (ti.input ?? ti.patch ?? "");
  if (input.tool_name === "apply_patch" && typeof patch === "string") {
    const re = /^\*\*\* (?:Add|Update|Delete) File: (.+)$|^\*\*\* Move to: (.+)$/gm;
    for (const m of patch.matchAll(re)) paths.push((m[1] ?? m[2]).trim());
  }
  return paths;
}

/** Путь относительно корня проекта с прямыми слешами. */
export function relPath(root, p) {
  const abs = isAbsolute(p) ? p : resolve(root, p);
  return relative(root, abs).split(sep).join("/");
}

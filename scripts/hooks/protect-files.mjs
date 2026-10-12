#!/usr/bin/env node
// PreToolUse: блокирует запись в защищённые файлы и любые shell-команды, трогающие .env.
// Работает и в Claude Code, и в Codex: блокировка = exit 2 + причина в stderr.
import { basename } from "node:path";
import { editedPaths, projectRoot, readHookInput, relPath } from "./lib.mjs";

const GENERATED = new Set([
  "src/fonts/font5x7_data.c",
  "src/fonts/icons_data.c",
  "src/fonts/icons.h",
]);
const LOCKFILES = new Set(["package-lock.json", "yarn.lock", "pnpm-lock.yaml"]);

function isEnvFile(name) {
  return name === ".env" || (name.startsWith(".env.") && name !== ".env.example");
}

/** Причина блокировки или null. */
function protectedReason(rel) {
  const name = basename(rel);
  if (isEnvFile(name)) return `${rel}: файлы окружения с секретами агенту менять запрещено`;
  if (rel === ".git" || rel.startsWith(".git/") || rel.includes("/.git/"))
    return `${rel}: служебная папка git, менять нельзя`;
  if (LOCKFILES.has(name))
    return `${rel}: lock-файл меняется только через npm install (с подтверждения человека)`;
  if (GENERATED.has(rel))
    return `${rel}: файл сгенерирован, правь tools/gen_font.py и запусти make fonts`;
  if (rel.startsWith("../")) return `${rel}: запись за пределами проекта запрещена`;
  return null;
}

// Упоминание .env в команде (кроме .env.example): cat .env, echo > .env, cp x .env.local ...
const ENV_IN_SHELL = /(^|[\s"'=/<>|;&(])\.env(?!\.example)(\.[\w.-]+)?(?=$|[\s"'|;&)<>])/;

function blockBash(command) {
  if (ENV_IN_SHELL.test(command)) {
    return "команда обращается к .env — чтение и изменение секретов агенту запрещено";
  }
  if (/\bgit\s+push\b[^|;&]*(--force\b|-f\b)/.test(command)) {
    return "git push --force запрещён";
  }
  return null;
}

function block(reason) {
  process.stderr.write(`[protect-files] Заблокировано: ${reason}\n`);
  process.exit(2);
}

const input = await readHookInput();
const root = projectRoot(input);

if (input.tool_name === "Bash") {
  const reason = blockBash(String(input.tool_input?.command ?? ""));
  if (reason) block(reason);
  process.exit(0);
}

for (const p of editedPaths(input)) {
  const reason = protectedReason(relPath(root, p));
  if (reason) block(reason);
}
process.exit(0);

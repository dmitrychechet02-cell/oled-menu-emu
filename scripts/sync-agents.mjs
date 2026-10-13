#!/usr/bin/env node
// Единый источник инструкций — .claude/. Этот скрипт раскладывает их для Codex:
//   .claude/skills/<name>/SKILL.md  →  .agents/skills/<name>/SKILL.md
//   .claude/rules/firmware.md       →  src/AGENTS.md   (вложенный AGENTS.md)
//   .claude/rules/web.md            →  web/AGENTS.md
//                                  →  tests/e2e/AGENTS.md
// node scripts/sync-agents.mjs          — синхронизировать
// node scripts/sync-agents.mjs --check  — только проверить (для CI), exit 1 при расхождении
import { existsSync, mkdirSync, readdirSync, readFileSync, writeFileSync } from "node:fs";
import { dirname, join, resolve } from "node:path";

const ROOT = resolve(import.meta.dirname, "..");
const check = process.argv.includes("--check");
const HEADER =
  "<!-- Сгенерировано scripts/sync-agents.mjs из .claude/. Правь исходник и запусти npm run sync:agents. -->\n\n";

const pairs = [];

const skillsDir = join(ROOT, ".claude/skills");
for (const name of readdirSync(skillsDir)) {
  pairs.push([
    join(skillsDir, name, "SKILL.md"),
    join(ROOT, ".agents/skills", name, "SKILL.md"),
    (s) => s,
  ]);
}

const stripFrontmatter = (s) => HEADER + s.replace(/^---\n[\s\S]*?\n---\n+/, "");
pairs.push([
  join(ROOT, ".claude/rules/firmware.md"),
  join(ROOT, "src/AGENTS.md"),
  stripFrontmatter,
]);
pairs.push([join(ROOT, ".claude/rules/web.md"), join(ROOT, "web/AGENTS.md"), stripFrontmatter]);
pairs.push([
  join(ROOT, ".claude/rules/web.md"),
  join(ROOT, "tests/e2e/AGENTS.md"),
  stripFrontmatter,
]);

let stale = 0;
for (const [src, dst, transform] of pairs) {
  const want = transform(readFileSync(src, "utf8"));
  const have = existsSync(dst) ? readFileSync(dst, "utf8") : null;
  if (have === want) continue;
  stale++;
  if (check) {
    console.error(`не синхронизировано: ${dst.slice(ROOT.length + 1)}`);
  } else {
    mkdirSync(dirname(dst), { recursive: true });
    writeFileSync(dst, want);
    console.log(`обновлено: ${dst.slice(ROOT.length + 1)}`);
  }
}

if (check && stale > 0) process.exit(1);
if (!check && stale === 0) console.log("всё уже синхронизировано");

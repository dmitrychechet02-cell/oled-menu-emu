#!/usr/bin/env node
// SessionStart (Claude Code): подмешивает в контекст новой сессии открытые пункты
// docs/plan.md и последнюю запись docs/memory.md — агент продолжает работу, а не начинает с нуля.
import { existsSync, readFileSync } from "node:fs";
import { join } from "node:path";
import { projectRoot, readHookInput } from "./lib.mjs";

const input = await readHookInput().catch(() => ({}));
const root = projectRoot(input);
const read = (p) => (existsSync(join(root, p)) ? readFileSync(join(root, p), "utf8") : "");

const plan = read("docs/plan.md");
const open = plan
  .split("\n")
  .filter((l) => /^\s*- \[ \]/.test(l))
  .slice(0, 10);

const memory = read("docs/memory.md");
const entries = memory.split(/\n(?=## )/);
const last = entries.length > 1 ? entries[entries.length - 1].trim() : "";

const parts = ["# Память проекта (из docs/plan.md и docs/memory.md)"];
parts.push(open.length ? `## Открытые пункты плана\n${open.join("\n")}` : "## План: всё выполнено");
if (last) parts.push(`## Последняя запись журнала\n${last}`);
parts.push(
  "Продолжай с первого открытого пункта плана. В конце сессии отметь сделанное в " +
    "docs/plan.md и допиши запись в docs/memory.md.",
);

process.stdout.write(`${parts.join("\n\n")}\n`);

// Реальные инструменты проверяют форматирование, ошибки и пути с пробелами.
import assert from "node:assert/strict";
import { spawnSync } from "node:child_process";
import { readFileSync, unlinkSync, writeFileSync } from "node:fs";
import { resolve } from "node:path";
import test from "node:test";

const root = resolve(import.meta.dirname, "../..");
const rel = `scripts/hook test ${process.pid}.mjs`;
const file = resolve(root, rel);
const invoke = (source) => {
  writeFileSync(file, source);
  const result = spawnSync(process.execPath, ["scripts/hooks/lint-after-edit.mjs"], {
    cwd: root,
    input: JSON.stringify({ cwd: root, tool_name: "Write", tool_input: { file_path: rel } }),
    encoding: "utf8",
  });
  assert.ifError(result.error);
  assert.equal(result.status, 0, result.stderr);
  return JSON.parse(result.stdout);
};

test("Prettier исправляет формат, ESLint проверяет файл с пробелами в имени", () => {
  try {
    const output = invoke("export const hookDemo={value:1}\n");
    assert.equal(output.decision, undefined);
    assert.match(output.systemMessage, /переформатирован/);
    assert.match(output.systemMessage, /eslint: .* ок/);
    assert.equal(readFileSync(file, "utf8"), "export const hookDemo = { value: 1 };\n");
  } finally {
    unlinkSync(file);
  }
});

test("ошибка Prettier возвращается агенту", () => {
  try {
    const output = invoke("export const = ;\n");
    assert.equal(output.decision, "block");
    assert.match(output.reason, /prettier/);
    assert.match(output.reason, /SyntaxError/);
  } finally {
    unlinkSync(file);
  }
});

test("ошибка ESLint возвращается агенту", () => {
  try {
    const output = invoke("export const hookDemo = unknownHookName;\n");
    assert.equal(output.decision, "block");
    assert.match(output.reason, /no-undef/);
  } finally {
    unlinkSync(file);
  }
});

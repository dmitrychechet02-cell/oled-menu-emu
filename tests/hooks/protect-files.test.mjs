// Проверяем решение hook и его передачу через оболочку без операций над файлами.
import assert from "node:assert/strict";
import { spawnSync } from "node:child_process";
import { resolve } from "node:path";
import test from "node:test";

const root = resolve(import.meta.dirname, "../..");
const hook = resolve(root, "scripts/hooks/protect-files.mjs");
const invoke = (event, shell) => {
  const result = spawnSync(
    shell ?? process.execPath,
    shell ? ["-NoProfile", "-Command", "node scripts/hooks/protect-files.mjs"] : [hook],
    { cwd: root, input: JSON.stringify(event), encoding: "utf8" },
  );
  assert.ifError(result.error);
  assert.equal(result.status, 0, result.stderr);
  return result.stdout.trim() ? JSON.parse(result.stdout) : null;
};

for (const path of [".env", ".env.local", ".git/config", "package-lock.json"]) {
  test(`защита пути ${path} без реальной записи`, () => {
    const output = invoke({
      cwd: root,
      tool_name: "apply_patch",
      tool_input: { command: `*** Begin Patch\n*** Add File: ${path}\n+demo\n*** End Patch` },
    });
    assert.equal(output.hookSpecificOutput.hookEventName, "PreToolUse");
    assert.equal(output.hookSpecificOutput.permissionDecision, "deny");
    assert.match(output.hookSpecificOutput.permissionDecisionReason, /Заблокировано/);
  });
}

test("обычный файл разрешён без реальной записи", () => {
  assert.equal(
    invoke({ cwd: root, tool_name: "Write", tool_input: { file_path: "docs/demo.md" } }),
    null,
  );
});

const shellEvent = {
  cwd: root,
  tool_name: "Bash",
  tool_input: { command: "node scripts/demo-env-block.mjs --demo-target=.env" },
};

test("shell-команда с .env запрещена JSON-решением", () => {
  assert.equal(invoke(shellEvent).hookSpecificOutput.permissionDecision, "deny");
});

if (process.platform === "win32") {
  for (const shell of ["powershell.exe", "pwsh.exe"]) {
    test(`решение deny сохраняется через ${shell}`, () => {
      assert.equal(invoke(shellEvent, shell).hookSpecificOutput.permissionDecision, "deny");
    });
  }
}

// Демонстрация защиты: передаём hook-у только описание операции, не выполняя её.
import { spawnSync } from "node:child_process";
import { resolve } from "node:path";

const root = resolve(import.meta.dirname, "..");
const hook = resolve(root, "scripts/hooks/protect-files.mjs");
const event = {
  cwd: root,
  tool_name: "apply_patch",
  tool_input: {
    command: "*** Begin Patch\n*** Add File: .env\n+DEMO_ONLY=1\n*** End Patch",
  },
};

console.log("Проверка защитного hook: тестовое событие добавления .env.");
console.log("Операция записи не выполняется; файл не читается и не создаётся.");
const blocked = spawnSync(process.execPath, [hook], {
  cwd: root,
  input: JSON.stringify(event),
  encoding: "utf8",
});
if (blocked.error) throw blocked.error;
const decision = JSON.parse(blocked.stdout).hookSpecificOutput;
console.log(decision.permissionDecisionReason);
console.log(`Код выхода hook: ${blocked.status}`);
if (
  blocked.status !== 0 ||
  decision.permissionDecision !== "deny" ||
  !decision.permissionDecisionReason.includes("[protect-files] Заблокировано: .env:")
) {
  console.error("FAIL: ожидаемая блокировка не подтверждена.");
  process.exit(1);
}
console.log("Решение PreToolUse: deny (запрет операции).");

// Контроль: обычный путь разрешён тем же hook-ом; запись также не выполняется.
const control = spawnSync(process.execPath, [hook], {
  cwd: root,
  input: JSON.stringify({
    ...event,
    tool_input: {
      command: "*** Begin Patch\n*** Add File: docs/demo-control.md\n+demo\n*** End Patch",
    },
  }),
  encoding: "utf8",
});
if (control.error) throw control.error;
if (control.status !== 0) {
  console.error("FAIL: контрольное событие с обычным путём не разрешено.");
  process.exit(1);
}
console.log("Контрольный путь docs/demo-control.md: разрешён (код 0), запись не выполнялась.");
console.log("PASS: hook блокирует .env и разрешает обычный документ.");
console.log("Это отдельный запуск скрипта, а не проверка автоматического вызова агентом.");

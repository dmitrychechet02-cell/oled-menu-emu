@AGENTS.md

## Дополнительно для Claude Code

- Правила по папкам лежат в `.claude/rules/` и подгружаются только для своих путей
  (`firmware.md` → `src/**`, `web.md` → `web/**`, `tests/e2e/**`).
- Навыки: `.claude/skills/` (`add-menu-screen`, `check-ui` — вызываешь сам;
  `release` — только по команде `/release`).
- Hooks (`.claude/settings.json`) сами форматируют и проверяют файл после правки и
  блокируют запись в защищённые файлы. Если hook вернул ошибку — исправь причину,
  не обходи hook.
- Для проверки UI в браузере используй Playwright MCP (`.mcp.json`).

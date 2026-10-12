# OLED 128×64 Menu Emulator

Эмулятор меню для монохромного OLED-дисплея 2.42" 128×64 на контроллере **SSD1309**.
Ядро на C (графика, шрифт 5×7 с кириллицей, движок меню, экраны) компилируется в
WebAssembly и работает в браузере, а тот же код без изменений собирается под микроконтроллер.
Железо не нужно: всё проверяется unit-тестами и в эмуляторе.

![Эмулятор](docs/screenshots/emulator.png)

## Быстрый старт

Нужны: `make`, `gcc`, `clang` + `lld` (с целью wasm32), `clang-format`, Node.js ≥ 20, Python 3
(только для перегенерации шрифта).

```bash
npm install
npx playwright install chromium
npm start                # собрать emu.wasm и открыть http://localhost:8080
```

| Команда               | Что делает                                        |
| --------------------- | ------------------------------------------------- |
| `make test`           | unit-тесты (38), `make test T=menu` — по подстроке |
| `npm run test:e2e`    | e2e в Chromium (5 сценариев)                      |
| `make lint`           | clang-format + сборка wasm и ARM с `-Werror`      |
| `npm run lint`        | всё: C + ESLint + Prettier                        |
| `make fonts`          | перегенерировать шрифт и иконки                   |

**Windows:** проще всего работать в WSL2 (Ubuntu): `sudo apt install build-essential clang lld clang-format`.
Через MSYS2 тоже работает (`pacman -S make mingw-w64-ucrt-x86_64-clang mingw-w64-ucrt-x86_64-lld`).

## Что внутри

```
src/gfx      фреймбуфер 1024 байта в раскладке SSD1309, линии, прямоугольники, текст UTF-8
src/fonts    шрифт 5×7 (ASCII + А–я, Ёё, °) и иконки — генерируются tools/gen_font.py
src/menu     движок меню: подменю, экраны, переключатели, значения, выбор, прокрутка
src/app      экраны и настройки, строки RU/EN
src/hal      display.h: ssd1309/ — для платы, web/ — для эмулятора
web/         страница эмулятора (canvas, кнопки ▲ ▼ OK ◄, клавиатура)
tests/       unit (свой мини-раннер) и e2e (Playwright)
docs/        план, журнал, отчёт по ДЗ
```

Экраны: **главное меню** → «Показания» (температура крупно, влажность, батарея),
«Настройки» (автосон, яркость = контраст SSD1309, язык RU/EN, инверсия, сброс),
«Символы» (вся таблица шрифта), «Об устройстве».

## Как агент работает в проекте

Проект настроен под **Claude Code** и **Codex** одновременно. Источник правды для инструкций —
`AGENTS.md` и папка `.claude/`; для Codex `npm run sync:agents` раскладывает навыки и правила
в его форматы (CI проверяет, что копии не разошлись).

### Какие инструкции он читает

| Файл                                | Кто читает                | Когда                                   |
| ----------------------------------- | ------------------------- | --------------------------------------- |
| `AGENTS.md`                         | Codex, Claude Code (через `CLAUDE.md`) | всегда: стек, команды, конвенции, запреты |
| `.claude/rules/firmware.md`         | Claude Code               | только при работе с `src/**/*.c,h`      |
| `.claude/rules/web.md`              | Claude Code               | только при работе с `web/**`, `tests/e2e/**` |
| `src/AGENTS.md`, `web/AGENTS.md`    | Codex                     | только внутри этих папок                |
| `.claude/skills/*`, `.agents/skills/*` | Claude Code / Codex    | навыки (см. ниже)                       |
| `docs/plan.md`, `docs/memory.md`    | оба                       | в начале и в конце каждой сессии        |

Навыки:

- `add-menu-screen` — агент вызывает сам: новый экран/пункт меню со строками, тестами и e2e;
- `check-ui` — агент вызывает сам: проверка эмулятора в браузере через Playwright MCP;
- `release` — только вручную: `/release 0.2.0` в Claude Code, `$release` в Codex.

### Какие у него права

| Уровень | Claude Code (`.claude/settings.json`) | Codex (`.codex/rules/project.rules`) |
| ------- | ------------------------------------- | ------------------------------------ |
| allow   | `make test/lint/wasm/format`, `npm run test:e2e/lint`, `git add/commit/diff/status`, Playwright MCP | те же команды через `prefix_rule(... "allow")` |
| ask     | `git push`, `git tag`, `git reset`, `git rebase`, `npm install` | `git push`, `reset/rebase/tag`, `npm install` → `"prompt"` |
| deny    | чтение `.env*` и `secrets/`, `rm -rf`, `sudo`, `git push --force`, `curl/wget` | `rm -rf`, `cat .env`, `git push --force`, `sudo` → `"forbidden"` |

**Режим по умолчанию:** Claude Code — `acceptEdits`, Codex — `workspace-write` +
`on-request`: правки файлов внутри репозитория идут без вопросов (их сразу страхуют hooks и
тесты), а любая команда вне allow-списка и всё необратимое требует подтверждения человека.

Личные настройки — в `.claude/settings.local.json` и `CLAUDE.local.md`, они в `.gitignore`.

### Какие проверки его страхуют

- **Hook перед записью** `scripts/hooks/protect-files.mjs` (PreToolUse, оба агента):
  блокирует запись в `.env*`, `.git/`, lock-файлы и сгенерированные файлы шрифта, а также
  shell-команды, которые трогают `.env`.
- **Hook после правки** `scripts/hooks/lint-after-edit.mjs` (PostToolUse, оба агента):
  C → `clang-format -i` + компиляция файла с `-Werror` под wasm32 freestanding (ловит и
  предупреждения, и случайный `#include` из libc); JS/CSS/HTML → Prettier + ESLint.
  Ошибки возвращаются агенту, и он обязан их исправить.
- **Hook начала сессии** `scripts/hooks/session-start.mjs` (Claude Code): подставляет открытые
  пункты плана и последнюю запись журнала — агент продолжает с того места, где остановился.
- **CI** (`.github/workflows/ci.yml`): синхронность инструкций, линтеры, unit и e2e.

### Как запустить тесты

```bash
make test                       # все unit-тесты
make test T=ssd1309             # тесты драйвера дисплея
npm run test:e2e                # e2e в браузере
npx playwright test -g "автосон"  # один e2e-сценарий
npx playwright show-report      # отчёт со скриншотами и трейсами
```

Подробный отчёт по домашнему заданию — [`docs/about.md`](docs/about.md).

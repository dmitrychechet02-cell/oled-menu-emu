---
name: check-ui
description: Проверить эмулятор OLED в настоящем браузере через Playwright MCP — пройти сценарий кнопками, сделать скриншот экрана и проверить консоль. Используй после изменений в src/app, src/menu, web/ или когда просят «посмотри, как выглядит».
---

# Проверка UI эмулятора в браузере

## 1. Собрать и запустить

```
make wasm
npm run serve        # http://localhost:8080, держи в фоне
```

## 2. Пройти сценарий через Playwright MCP

1. `browser_navigate` → `http://localhost:8080/?paused=1` (время остановлено).
2. Дождись, пока у `<body>` появится `data-ready="true"`.
3. Нажимай кнопки: `browser_click` по `[data-testid="key-down"]`, `key-ok`, `key-up`,
   `key-back`, либо `browser_press_key` (`ArrowDown`, `Enter`, `Escape`).
4. После каждого шага сверяй заголовок в `[data-testid="state-title"]`.
5. Для точных проверок: `browser_evaluate` → `window.emu.state()` (экран, пункт,
   контраст) и `window.emu.pixel(x, y)` (пиксель RAM дисплея). Время —
   `window.emu.tick(ms)`.

## 3. Зафиксировать результат

- `browser_take_screenshot` элемента `[data-testid="screen"]` →
  `docs/screenshots/<экран>.png`.
- `browser_console_messages` — ошибок и предупреждений быть не должно.

## 4. Закрепить тестом

Если сценарий новый — перенеси его в `tests/e2e/menu.spec.js` и доведи
`npm run test:e2e` до зелёного.

## Если что-то не так

- Пустой экран и красная плашка «Не удалось загрузить emu.wasm» → не выполнен `make wasm`.
- Кириллица квадратиками → символа нет в шрифте, добавь глиф в `tools/gen_font.py`.

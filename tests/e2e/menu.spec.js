// E2E: настоящее C-ядро (WebAssembly) в браузере, сценарии нажатия кнопок.
import { expect, test } from "@playwright/test";

/** Открыть эмулятор с остановленным временем и собирать ошибки консоли. */
async function openEmulator(page) {
  const problems = [];
  page.on("console", (msg) => {
    if (msg.type() === "error" || msg.type() === "warning") problems.push(msg.text());
  });
  page.on("pageerror", (err) => problems.push(err.message));
  await page.goto("/?paused=1");
  await expect(page.locator("body")).toHaveAttribute("data-ready", "true");
  return problems;
}

const state = (page) => page.evaluate(() => window.emu.state());

test("главное меню рисуется на экране без ошибок в консоли", async ({ page }) => {
  const problems = await openEmulator(page);

  await expect(page.getByTestId("state-title")).toHaveText("Главное меню");
  const s = await state(page);
  expect(s.screen).toBe("menu");
  expect(s.powered).toBe(true);

  // Шапка залита (инверсный заголовок), первая строка выделена.
  const px = await page.evaluate(() => [window.emu.pixel(0, 0), window.emu.pixel(0, 13)]);
  expect(px).toEqual([1, 1]);

  // Canvas действительно что-то нарисовал: в области экрана есть светлые пиксели.
  const lit = await page.evaluate(() => {
    const c = document.getElementById("screen");
    const d = c.getContext("2d").getImageData(0, 0, c.width, c.height).data;
    let n = 0;
    for (let i = 0; i < d.length; i += 4) if (d[i] > 100) n++;
    return n;
  });
  expect(lit).toBeGreaterThan(1000);

  expect(problems).toEqual([]);
});

test("навигация: Настройки → Яркость → изменение контраста", async ({ page }) => {
  const problems = await openEmulator(page);

  await page.getByTestId("key-down").click();
  await page.getByTestId("key-ok").click();
  await expect(page.getByTestId("state-title")).toHaveText("Настройки");

  await page.keyboard.press("ArrowDown"); // Яркость
  await page.keyboard.press("Enter"); // режим правки
  expect((await state(page)).editing).toBe(true);

  const before = (await state(page)).contrast;
  await page.keyboard.press("ArrowDown");
  await page.keyboard.press("ArrowDown");
  const s = await state(page);
  expect(s.brightness).toBe(6);
  expect(s.contrast).toBeLessThan(before);

  await page.keyboard.press("Escape"); // выход из правки
  await page.keyboard.press("Escape"); // назад в главное меню
  await expect(page.getByTestId("state-title")).toHaveText("Главное меню");
  expect(problems).toEqual([]);
});

test("смена языка переключает все надписи на английский", async ({ page }) => {
  const problems = await openEmulator(page);
  await page.evaluate(() => {
    const { press } = window.emu;
    press("down");
    press("ok"); // Настройки
    press("down");
    press("down"); // Язык
    press("ok"); // RU -> EN
  });
  await expect(page.getByTestId("state-title")).toHaveText("Settings");
  await page.evaluate(() => window.emu.press("back"));
  await expect(page.getByTestId("state-title")).toHaveText("Main menu");
  expect(problems).toEqual([]);
});

test("экран показаний и таблица символов", async ({ page }) => {
  const problems = await openEmulator(page);

  await page.keyboard.press("Enter");
  await expect(page.getByTestId("state-title")).toHaveText("Показания");
  expect((await state(page)).screen).toBe("readings");
  await page.keyboard.press("Escape");

  await page.keyboard.press("ArrowDown");
  await page.keyboard.press("ArrowDown");
  await page.keyboard.press("Enter");
  await expect(page.getByTestId("state-title")).toHaveText("Символы 1/2");
  await page.keyboard.press("ArrowDown");
  await expect(page.getByTestId("state-title")).toHaveText("Символы 2/2");

  await page.getByTestId("screen").screenshot({ path: "test-results/symbols-cyrillic.png" });
  expect(problems).toEqual([]);
});

test("автосон через 30 с бездействия и пробуждение кнопкой", async ({ page }) => {
  const problems = await openEmulator(page);
  await page.evaluate(() => window.emu.tick(30000));
  await expect(page.getByTestId("state-power")).toHaveText("спит");

  await page.getByTestId("key-down").click(); // только будит
  await expect(page.getByTestId("state-power")).toHaveText("включён");
  expect((await state(page)).selected).toBe(0);
  expect(problems).toEqual([]);
});

(function () {
  // Основное состояние симулятора
  let ups = {
    mode: 'OFF',
    on: false,
    onMin: 5,
    offMin: 5,
    voltage: 12.6,
    charging: false,
    next: -1,
    protectedFromMode: null,
    lastSwitch: Date.now(),

    // Wi-Fi настройки
    ssid: 'Smart_UPS',
    pass: '12345678',

    // Пороги батареи (используются в low/critical и в battery.html)
    vLow: 12.1,
    vCrit: 11.9,
    vRec: 12.5
  };

  const V_MAX = 14.2;

  const clamp = (v, min, max) => Math.min(max, Math.max(min, v));

  const hms = (s) => {
    if (s < 0) return '--:--:--';
    const h = Math.floor(s / 3600);
    const m = Math.floor((s % 3600) / 60);
    const sec = s % 60;
    return `${h.toString().padStart(2, '0')}:${m.toString().padStart(2, '0')}:${sec.toString().padStart(2, '0')}`;
  };

  // Перехват всех fetch-запросов
  const originalFetch = window.fetch;
  window.fetch = async function (url, opts) {
    if (url.startsWith('/status')) {
      const batPercent = Math.round((ups.voltage - 11.6) / (12.8 - 11.6) * 100);
      return Promise.resolve(new Response(JSON.stringify({
        ups: ups.on,
        mode: ups.mode,
        on: ups.onMin,
        off: ups.offMin,
        next: ups.next,
        vin: Number(ups.voltage.toFixed(2)),
        bat: Math.max(0, Math.min(100, batPercent)),
        low: ups.voltage < ups.vLow && ups.voltage >= ups.vCrit,
        critical: ups.voltage < ups.vCrit,
        charging: !ups.on,

        // Для settings.html
        ssid: ups.ssid,
        pass: ups.pass,

        // Для battery.html
        vLow: ups.vLow,
        vCrit: ups.vCrit,
        vRec: ups.vRec
      }), {
        status: 200,
        headers: { 'Content-Type': 'application/json' }
      }));
    }

    if (url.startsWith('/mode')) {
      const m = url.split('=')[1];
      setMode(m);
      return Promise.resolve(new Response("OK", { status: 200 }));
    }

    if (url.startsWith('/save')) {
      const params = new URLSearchParams(url.split('?')[1]);
      ups.onMin = parseInt(params.get('on')) || ups.onMin;
      ups.offMin = parseInt(params.get('off')) || ups.offMin;
      return Promise.resolve(new Response("OK", { status: 200 }));
    }

    if (url.startsWith('/setadc')) {
      const params = new URLSearchParams(url.split('?')[1]);
      const vLow = parseFloat(params.get('vLow'));
      const vCrit = parseFloat(params.get('vCrit'));
      const vRec = parseFloat(params.get('vRec'));

      if (!isNaN(vLow)) ups.vLow = vLow;
      if (!isNaN(vCrit)) ups.vCrit = vCrit;
      if (!isNaN(vRec)) ups.vRec = vRec;

      // Простая коррекция значений (как на странице)
      if (ups.vCrit > ups.vRec) ups.vRec = ups.vCrit;
      if (ups.vLow < ups.vCrit) ups.vLow = ups.vCrit;

      return Promise.resolve(new Response("OK", { status: 200 }));
    }

    if (url.startsWith('/setwifi')) {
      const params = new URLSearchParams(url.split('?')[1]);
      ups.ssid = decodeURIComponent(params.get('ssid') || ups.ssid);
      ups.pass = decodeURIComponent(params.get('pass') || ups.pass);
      return Promise.resolve(new Response("OK", { status: 200 }));
    }

    // Заглушки для остальных эндпоинтов
    if (url.startsWith('/toggleap') || url.startsWith('/apmode') || url.startsWith('/ota')) {
      return Promise.resolve(new Response("OK", { status: 200 }));
    }

    // Все остальные запросы пропускаем (на случай реального сервера)
    return originalFetch.apply(this, arguments);
  };

  // Переключение режима
  function setMode(m) {
    if (m == 0 || m === '0') m = 'OFF';
    else if (m == 1 || m === '1') m = 'ON';
    else if (m == 2 || m === '2') m = 'CYCLE';

    ups.mode = m;
    ups.on = (m === 'ON' || m === 'CYCLE');
    ups.protectedFromMode = null;
    ups.lastSwitch = Date.now();

    if (m === 'CYCLE') {
      ups.next = ups.onMin * 60;
    } else {
      ups.next = -1;
    }

    updateButtonActive();
  }

  function updateButtonActive() {
    document.getElementById('on')?.classList.toggle('active', ups.mode === 'ON');
    document.getElementById('off')?.classList.toggle('active', ups.mode === 'OFF');
    document.getElementById('cyc')?.classList.toggle('active', ups.mode === 'CYCLE');
  }

  // Симуляция физики и логики
  function simulate() {
    const now = Date.now();

    // Защита от глубокого разряда
    if (ups.voltage < ups.vCrit) {
      if (ups.mode !== 'OFF') ups.protectedFromMode = ups.mode;
      ups.mode = 'OFF';
      ups.on = false;
      ups.next = -1;
    } else if (ups.voltage >= ups.vRec && ups.protectedFromMode) {
      ups.mode = ups.protectedFromMode;
      ups.on = ups.mode === 'ON';
      ups.protectedFromMode = null;
      ups.lastSwitch = now;
    }

    // Изменение напряжения
    ups.voltage += ups.on ? -0.03 : 0.05;
    ups.voltage = clamp(ups.voltage, 10.8, V_MAX);
    ups.charging = !ups.on;

    // Логика цикла
    if (ups.mode === 'CYCLE') {
      const periodMs = (ups.on ? ups.onMin : ups.offMin) * 60000;
      const elapsed = now - ups.lastSwitch;
      ups.next = Math.max(0, Math.floor((periodMs - elapsed) / 1000));

      if (elapsed >= periodMs) {
        ups.on = !ups.on;
        ups.lastSwitch = now;
        ups.next = (ups.on ? ups.onMin : ups.offMin) * 60;
      }
    } else {
      ups.next = -1;
    }
  }

  // Обновление интерфейса главной страницы
  function updateUI() {
    const $ = id => document.getElementById(id);
    if (!$('ups')) return; // если не на главной странице — выходим

    $('ups').innerText = ups.on ? 'ON' : 'OFF';
    $('mode').innerText = ups.mode;
    $('next').innerText = hms(ups.next);
    $('vin').innerText = ups.voltage.toFixed(2);

    const perc = Math.round((ups.voltage - 11.6) / (12.8 - 11.6) * 100);
    $('bat').innerText = Math.max(0, Math.min(100, perc)) + '%';
    $('dir').innerText = ups.charging ? '↑ Charging' : '↓ Discharging';

    const pwr = $('pwr');
    pwr.className = '';
    if (ups.voltage < ups.vCrit) {
      pwr.innerText = 'CRITICAL';
      pwr.classList.add('crit');
    } else if (ups.voltage < ups.vLow) {
      pwr.innerText = 'LOW';
      pwr.classList.add('low');
    } else {
      pwr.innerText = 'OK';
      pwr.classList.add('ok');
    }

    // Обновляем слайдеры, если пользователь не трогает
    if ($('onRange') && !document.activeElement?.matches('#onRange')) {
      $('onRange').value = ups.onMin;
      $('onVal').textContent = ups.onMin;
    }
    if ($('offRange') && !document.activeElement?.matches('#offRange')) {
      $('offRange').value = ups.offMin;
      $('offVal').textContent = ups.offMin;
    }
  }

  // Привязка событий к кнопкам (только один раз)
  document.addEventListener('DOMContentLoaded', () => {
    const btns = {
      on: 1,
      off: 0,
      cyc: 2
    };

    Object.entries(btns).forEach(([id, value]) => {
      const btn = document.getElementById(id);
      if (btn) {
        btn.addEventListener('click', () => setMode(value));
      }
    });

    updateButtonActive();
  });

  // Запуск симуляции
  setInterval(() => {
    simulate();
    updateUI();
  }, 400);

})();
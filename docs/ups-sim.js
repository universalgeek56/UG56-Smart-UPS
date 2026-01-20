(function () {
  let ups = {
    mode: "OFF",
    on: false,
    onMin: 5,
    offMin: 5,
    voltage: 12.6,
    charging: false,
    next: -1,
    protectedFromMode: null,
    lastSwitch: Date.now()
  };

  const V_LOW = 12.1;
  const V_CRIT = 11.9;
  const V_UPPER = 12.5;
  const V_MAX = 14.2;

  const clamp = (v, a, b) => Math.min(b, Math.max(a, v));

  function hms(s) {
    if (s < 0) return "--:--:--";
    let h = Math.floor(s / 3600);
    let m = Math.floor((s % 3600) / 60);
    let sec = s % 60;
    return `${h.toString().padStart(2, "0")}:${m
      .toString()
      .padStart(2, "0")}:${sec.toString().padStart(2, "0")}`;
  }

  // Перехват fetch
  const originalFetch = window.fetch;
  window.fetch = async function (url, opts) {
    if (url.startsWith("/status")) {
      const batPercent = Math.round(
        ((ups.voltage - 11.6) / (12.8 - 11.6)) * 100
      );
      return Promise.resolve(
        new Response(
          JSON.stringify({
            ups: ups.on,
            mode: ups.mode,
            on: ups.onMin,
            off: ups.offMin,
            next: ups.next,
            vin: Number(ups.voltage.toFixed(2)),
            bat: Math.max(0, Math.min(100, batPercent)),
            low: ups.voltage < V_LOW && ups.voltage >= V_CRIT,
            critical: ups.voltage < V_CRIT,
            charging: !ups.on
          }),
          { status: 200, headers: { "Content-Type": "application/json" } }
        )
      );
    }

    if (url.startsWith("/mode")) {
      const m = url.split("=")[1];
      setMode(m);
      return Promise.resolve(new Response("OK", { status: 200 }));
    }

    if (url.startsWith("/save")) {
      const params = new URLSearchParams(url.split("?")[1]);
      ups.onMin = parseInt(params.get("on")) || ups.onMin;
      ups.offMin = parseInt(params.get("off")) || ups.offMin;
      return Promise.resolve(new Response("OK", { status: 200 }));
    }

    return originalFetch.apply(this, arguments);
  };

  function setMode(m) {
    if (m == 0 || m === "0") m = "OFF";
    else if (m == 1 || m === "1") m = "ON";
    else if (m == 2 || m === "2") m = "CYCLE";

    ups.mode = m;
    ups.on = m === "ON" || m === "CYCLE";
    ups.protectedFromMode = null;
    ups.lastSwitch = Date.now();

    if (m === "CYCLE") {
      ups.next = ups.onMin * 60;
    } else {
      ups.next = -1;
    }

    updateButtonActive();
  }

  function updateButtonActive() {
    document
      .getElementById("on")
      ?.classList.toggle("active", ups.mode === "ON");
    document
      .getElementById("off")
      ?.classList.toggle("active", ups.mode === "OFF");
    document
      .getElementById("cyc")
      ?.classList.toggle("active", ups.mode === "CYCLE");
  }

  function simulate() {
    const now = Date.now();

    if (ups.voltage < V_CRIT) {
      if (ups.mode !== "OFF") ups.protectedFromMode = ups.mode;
      ups.mode = "OFF";
      ups.on = false;
      ups.next = -1;
    } else if (ups.voltage >= V_UPPER && ups.protectedFromMode) {
      ups.mode = ups.protectedFromMode;
      ups.on = ups.mode === "ON";
      ups.protectedFromMode = null;
      ups.lastSwitch = now;
    }

    ups.voltage += ups.on ? -0.03 : 0.05;
    ups.voltage = clamp(ups.voltage, 10.8, V_MAX);
    ups.charging = !ups.on;

    if (ups.mode === "CYCLE") {
      const periodMs = (ups.on ? ups.onMin : ups.offMin) * 60_000;
      const elapsed = now - ups.lastSwitch;

      ups.next = Math.max(0, Math.floor((periodMs - elapsed) / 1000));

      if (elapsed >= periodMs) {
        ups.on = !ups.on;
        ups.lastSwitch = now;
        // можно сразу обновить next
        ups.next = (ups.on ? ups.onMin : ups.offMin) * 60;
      }
    } else {
      ups.next = -1;
    }
  }

  function updateUI() {
    const $ = (id) => document.getElementById(id);

    $("ups").innerText = ups.on ? "ON" : "OFF";
    $("mode").innerText = ups.mode;
    $("next").innerText = hms(ups.next);
    $("vin").innerText = ups.voltage.toFixed(2);

    const perc = Math.round(((ups.voltage - 11.6) / (12.8 - 11.6)) * 100);
    $("bat").innerText = Math.max(0, Math.min(100, perc)) + "%";
    $("dir").innerText = ups.charging ? "↑ Charging" : "↓ Discharging";

    const pwr = $("pwr");
    pwr.className = "";
    if (ups.voltage < V_CRIT) {
      pwr.innerText = "CRITICAL";
      pwr.classList.add("crit");
    } else if (ups.voltage < V_LOW) {
      pwr.innerText = "LOW";
      pwr.classList.add("low");
    } else {
      pwr.innerText = "OK";
      pwr.classList.add("ok");
    }

    if ($("onRange")) {
      $("onRange").value = $("onVal").innerText = ups.onMin;
    }
    if ($("offRange")) {
      $("offRange").value = $("offVal").innerText = ups.offMin;
    }
  }

  document.addEventListener("DOMContentLoaded", () => {
    document.getElementById("on")?.addEventListener("click", () => setMode(1));
    document.getElementById("off")?.addEventListener("click", () => setMode(0));
    document.getElementById("cyc")?.addEventListener("click", () => setMode(2));

    updateButtonActive();
  });

  setInterval(() => {
    simulate();
    updateUI();
  }, 1000); 
})();

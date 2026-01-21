// footer.js 
(function () {
  if (document.querySelector('.sim-footer')) return;

  document.addEventListener('DOMContentLoaded', () => {
    const footer = document.createElement('div');
    footer.className = 'sim-footer';
    footer.innerHTML = `
      <svg id="sim-logo" viewBox="0 0 24 24" aria-label="UG56 Logo" role="img">
        <defs>
          <filter id="glow" x="-50%" y="-50%" width="200%" height="200%">
            <feGaussianBlur stdDeviation="1.5" result="blur"/>
            <feMerge>
              <feMergeNode in="blur"/>
              <feMergeNode in="SourceGraphic"/>
            </feMerge>
          </filter>
        </defs>
        <g filter="url(#glow)">
          <line x1="12" y1="4" x2="12" y2="10.5" class="line"/>
          <path d="M16.24 7a7 7 0 1 1-8.48 0" class="circle"/>
        </g>
      </svg>
      <span>UG56 © 2026 MIT License</span>
    `;

    if (!document.getElementById('sim-footer-styles')) {
      const style = document.createElement('style');
      style.id = 'sim-footer-styles';
      style.textContent = `
        .sim-footer {
          display: flex;
          align-items: center;
          gap: 8px;
          font-size: 12px;
          color: #888;
          margin: 16px auto 20px; 
          justify-content: center;
          opacity: 0.75;
          max-width: 420px;
          width: 100%;
          padding: 0 12px;
          box-sizing: border-box;
        }
        .sim-footer svg {
          width: 24px;
          height: 24px;
          color: #4CAF50;
          flex-shrink: 0;
        }
        .sim-footer span {
          white-space: nowrap;
        }
        .line, .circle {
          stroke: currentColor;
          stroke-width: 2;
          fill: none;
          stroke-dasharray: 100;
          stroke-dashoffset: 100;
          opacity: 0;
          animation-timing-function: ease-out;
          animation-fill-mode: forwards;
        }
        .animate .circle {
          animation: drawFade 4.8s backwards;
        }
        .animate .line {
          animation: drawFade 4s 0.4s backwards;
        }
        @keyframes drawFade {
          0%   { stroke-dashoffset: 100; opacity: 0; }
          20%  { opacity: 1; }
          70%  { stroke-dashoffset: 0; opacity: 0.75; }
          100% { opacity: 0; }
        }
      `;
      document.head.appendChild(style);
    }

    const container = document.querySelector('.wrap') || document.body;
    container.appendChild(footer);

    const logo = document.getElementById('sim-logo');
    if (logo) {
      setTimeout(() => {
        logo.classList.add('animate');
      }, 400);

      logo.addEventListener('mouseenter', () => {
        logo.classList.remove('animate');
        void logo.offsetWidth; // reflow
        setTimeout(() => logo.classList.add('animate'), 80);
      });
    }
  });
})();
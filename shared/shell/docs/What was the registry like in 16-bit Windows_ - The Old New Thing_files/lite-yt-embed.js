/*! lite-yt-embed  MIT License  (c) Paul Irish  https://github.com/paulirish/lite-youtube-embed */
/**
 * A lightweight youtube embed. Renders a static thumbnail + play button; loads
 * the heavy YouTube iframe (and its ~660 KiB of player scripts) only on click.
 */
class LiteYTEmbed extends HTMLElement {
    connectedCallback() {
        this.videoId = this.getAttribute('videoid');

        let playBtnEl = this.querySelector('.lty-playbtn');
        // A label for the button takes priority over a [playlabel] attribute.
        this.playLabel = (playBtnEl && playBtnEl.textContent.trim()) || this.getAttribute('playlabel') || 'Play';

        // Fall back to hqdefault if a poster wasn't set inline via style.
        if (!this.style.backgroundImage) {
            this.style.backgroundImage = `url("https://i.ytimg.com/vi/${this.videoId}/hqdefault.jpg")`;
        }

        if (!playBtnEl) {
            playBtnEl = document.createElement('button');
            playBtnEl.type = 'button';
            playBtnEl.classList.add('lty-playbtn');
            this.append(playBtnEl);
        }
        if (!playBtnEl.textContent) {
            const playBtnLabelEl = document.createElement('span');
            playBtnLabelEl.className = 'lyt-visually-hidden';
            playBtnLabelEl.textContent = this.playLabel;
            playBtnEl.append(playBtnLabelEl);
        }

        this.addNoscriptIframe();

        // Warm the connection to YouTube's origins on first hover/focus.
        this.addEventListener('pointerover', LiteYTEmbed.warmConnections, { once: true });
        this.addEventListener('focusin', LiteYTEmbed.warmConnections, { once: true });

        this.addEventListener('click', () => this.addIframe());
    }

    static addPrefetch(kind, url) {
        const linkEl = document.createElement('link');
        linkEl.rel = kind;
        linkEl.href = url;
        document.head.append(linkEl);
    }

    static warmConnections() {
        if (LiteYTEmbed.preconnected) return;
        LiteYTEmbed.addPrefetch('preconnect', 'https://www.youtube-nocookie.com');
        LiteYTEmbed.addPrefetch('preconnect', 'https://www.google.com');
        LiteYTEmbed.addPrefetch('preconnect', 'https://googleads.g.doubleclick.net');
        LiteYTEmbed.addPrefetch('preconnect', 'https://static.doubleclick.net');
        LiteYTEmbed.preconnected = true;
    }

    fetchYTPlayerApi() {
        // If the API is already loaded, ensure callers can still await safely.
        if (window.YT && window.YT.Player) {
            this.ytApiPromise = this.ytApiPromise || Promise.resolve();
            return;
        }

        // If a load is already in progress (or YT exists but Player isn't ready yet),
        // reuse the promise instead of returning without setting it.
        if (this.ytApiPromise) return;

        if (window.YT && typeof window.YT.ready === 'function') {
            this.ytApiPromise = new Promise((res) => { window.YT.ready(res); });
            return;
        }

        this.ytApiPromise = new Promise((res, rej) => {
            const el = document.createElement('script');
            el.src = 'https://www.youtube.com/iframe_api';
            el.async = true;
            el.onload = () => { window.YT.ready(res); };
            el.onerror = rej;
            this.append(el);
        });
    }

    async addYTPlayerIframe(params) {
        this.fetchYTPlayerApi();
        await this.ytApiPromise;
        const videoPlaceholderEl = document.createElement('div');
        this.append(videoPlaceholderEl);
        const paramsObj = Object.fromEntries(params.entries());
        new YT.Player(videoPlaceholderEl, {
            width: '100%',
            videoId: this.videoId,
            playerVars: paramsObj,
            events: { onReady: (event) => { event.target.playVideo(); } },
        });
    }

    addIframe() {
        if (this.classList.contains('lyt-activated')) return;
        this.classList.add('lyt-activated');

        const params = new URLSearchParams(this.getAttribute('params') || []);
        params.append('autoplay', '1');
        params.append('playsinline', '1');

        if (this.getAttribute('js-api') === 'true') {
            this.addYTPlayerIframe(params);
            return;
        }

        const iframeEl = document.createElement('iframe');
        iframeEl.width = 560;
        iframeEl.height = 315;
        iframeEl.title = this.playLabel;
        iframeEl.allow = 'accelerometer; autoplay; encrypted-media; gyroscope; picture-in-picture; web-share';
        iframeEl.allowFullscreen = true;
        iframeEl.src = `https://www.youtube-nocookie.com/embed/${encodeURIComponent(this.videoId)}?${params.toString()}`;
        this.append(iframeEl);

        // Move focus into the iframe for keyboard users.
        iframeEl.focus();
    }

    addNoscriptIframe() {
        const iframeEl = this.querySelector('iframe');
        if (iframeEl) return; // already has one
    }
}

customElements.define('lite-youtube', LiteYTEmbed);

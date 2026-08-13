// For compatibility with optimizers
if (document.readyState !== 'loading') {
    //console.log('Newsletter loaded (late)');
    tnp_ajax_init();
} else {
    document.addEventListener("DOMContentLoaded", function () {
        //console.log('Newsletter loaded');
        tnp_ajax_init();
    });
}

function tnp_ajax_init() {
    document.querySelectorAll('form.tnp-ajax').forEach(el => {
        el.addEventListener('submit', async function(ev) {
            ev.preventDefault();
            ev.stopPropagation();
            const response = await fetch(newsletter_data.action_url + '?action=tnp&na=sa', {
                method: "POST",
                body: new FormData(this)
            });
            this.innerHTML = await response.text();
        });
    });
}




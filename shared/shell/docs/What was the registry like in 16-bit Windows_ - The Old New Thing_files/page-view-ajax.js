document.addEventListener('DOMContentLoaded', function () {
    const postId = PostViewsAjaxData.post_id;
    var nonce = PostViewsAjaxData.nonce;

    if (postId) {
        // Increment the view count
        jQuery.ajax({
            url: PostViewsAjaxData.ajax_url,
            type: 'POST',
            data: {
                action: 'manage_post_views',
                action_type: 'set_and_get', // Handle both increment and retrieval
                post_id: postId,
                nonce: nonce
            },
            success: function (response) {
                if (response.success && response.data.count !== undefined) {
                    const viewCountElement = document.getElementById('post-view-count');
                    if (viewCountElement) {
                        viewCountElement.textContent = `${response.data.count} views`;
                    }
                }
            },
            error: function (xhr, status, error) {
                console.error('Error updating or fetching post views:', error);
            }
        });
    }
});
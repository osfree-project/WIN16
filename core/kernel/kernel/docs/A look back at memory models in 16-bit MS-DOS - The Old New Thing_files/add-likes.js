function post_like_vote_add(post_id, nonce, userid, blog_id, emoji = "heart") {
    // Hide tooltip
    jQuery('[data-toggle="tooltip"]').tooltip("hide");

    var closeDropdown = jQuery.Event('keydown', {
        'key': 'Escape',
        'which': 27,
        'keyCode': 27
    });
    var parent = jQuery('#likesMenu .evo-dropdown-button');
    parent.focus().trigger(closeDropdown);
    parent.hide();

    var self = "#postlike-" + post_id + " span button";

    // Create loader container
    var loaderContainer = jQuery('<span/>', {
        'class': 'loader-image-container'
    }).css({
        'display': 'flex',
        'height': '64px',
        'align-items': 'center',
        'margin': 'auto'
    }).insertAfter(self);


    // Create loader image
    var loader = jQuery('<img/>', {
        src: devblog_url.siteurl + '/wp-admin/images/loading.gif',
        'class': 'loader-image'
    }).appendTo(loaderContainer);

    // Display loader before AJAX request
    loaderContainer.show();

    // AJAX request
    jQuery.ajax({
        type: 'POST',
        url: devblogs_ajax.ajaxurl,
        data: {
            action: 'doPostLikeDislike',
            userid: userid,
            postid: post_id,
            nonce: nonce,
            blogid: blog_id,
            is_single: ajax_object.is_single,
            emoji: emoji
        },
        success: function (data, textStatus, XMLHttpRequest) {
            var response = JSON.parse(data);
            // Hide loader after successful response
            loaderContainer.hide();
            parent.show();

            var postReactButton = "#postlike-" + post_id + " span button";
            jQuery(postReactButton)
                .html(response.image + '<span class="votes-count fw-600">' + response.count + '</span>')
                .attr('data-bs-original-title', response.title)
                .attr('data-bi-name', response.title)
                .attr('data-bi-id', 'post_page_sidebar_left_social_' + response.title)
                .attr('aria-label', response.title + ' vote count ' + response.count);

            var reactsMiniCount = jQuery('#reactionsMini .reacts-count');
            var reactImages = jQuery('#reactionsMini .reacts-icons');

            if (response.count > 0) {
                jQuery(reactImages).html('');
                jQuery(reactImages).html(response.emojiImages);
                jQuery(reactImages).show();
            } else {
                jQuery(reactImages).html('');
                jQuery(reactImages).hide();
            }

            jQuery(reactsMiniCount).html('');
            jQuery(reactsMiniCount).html(response.count + ' reaction' + response.plural);

            jQuery('#likesMenu .evo-dropdown-item').removeClass('selected');
            if (!response.removedReaction) {
                jQuery('#likesMenu .evo-dropdown-item[data-react="' + emoji + '"]').addClass('selected');
            }
        },
        error: function (MLHttpRequest, textStatus, errorThrown) {
            // Hide loader on error (optional)
            loaderContainer.hide();
            parent.show();

            // Display error message (e.g., alert)
            alert(errorThrown);
        }
    });
}

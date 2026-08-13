// JavaScript Document
function commentsvote_add(comment_id, nonce, userid) {
    var self = "#commentsvote-" + comment_id + " span.icon-like-dislike a .vote_counts";
    // Hide tooltip
    jQuery('[data-toggle="tooltip"]').tooltip("hide");
    // var loaderContainer = jQuery( '<span/>', {
    //     'class': 'loader-image-container'
    // }).insertAfter(self);

    // var loader = jQuery( '<img/>', {
    //     src: siteurl.devblogsiteurl + '/wp-admin/images/loading.gif',
    //     'class': 'loader-image'
    // }).appendTo( loaderContainer );

    var loaderContainer = jQuery('<span/>', {
        'class': 'loader_center'
    }).insertAfter(self);

    var loader = jQuery('<span/>', {
        'class': 'loader'
    }).appendTo(loaderContainer);

    jQuery.ajax({
        type: 'POST',
        url: votecommentajax.ajaxurl,
        data: {
            action: 'doCertainThings',
            userid: userid,
            commentid: comment_id,
            postid: devblogsPost.ID,
            nonce: nonce
        },
        success: function (data, textStatus, XMLHttpRequest) {
            loaderContainer.remove();
            var linkofcomment = '#commentsvote-' + comment_id + ' span';
            jQuery(linkofcomment).html('');
            jQuery(linkofcomment).append(data);
            jQuery('[data-toggle="tooltip"]').not('[data-original-title]').tooltip();
        },
        error: function (MLHttpRequest, textStatus, errorThrown) {
            alert(errorThrown);
        }
    });
    loadSurveyMonkeyScript();
}

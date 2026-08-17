// Social sidebar position and sticky
document.addEventListener('DOMContentLoaded', function () {
    // Attach click event to anchor links
    document.querySelectorAll('.social-panel a.share-post').forEach(function(shareButton) {
      shareButton.addEventListener('click', function (event) {
        event.preventDefault();
        var postID = shareButton.getAttribute('data-post-id');
        var href = shareButton.getAttribute('href'); // Get the href URL
        
        // Send AJAX request
        var xhr = new XMLHttpRequest();
        var xhrData = new FormData();
  
        xhrData.append('action', 'increase_share_count');
        xhrData.append('post_id', postID);
  
        xhr.onreadystatechange = function() {
          if (xhr.readyState === XMLHttpRequest.DONE) {
            const status = xhr.status;
            // on success
            if (status === 0 || (status >= 200 && status < 400)) {
              // Update share count display
              document.querySelector('.evo-count-shares[data-post-id="' + postID + '"]').innerHTML = xhr.responseText;
            // on any other state
            } else {
              console.log(xhr.responseText);
            }
          }
        };
  
        xhr.open('POST',  devblogs_ajax_evo.ajaxurl);
        xhr.send(xhrData);

        var newWindow = window.open(href, '_blank');
        if (!newWindow) {
          console.error('Failed to open new window. Please check your popup settings.');
        }
      });
    });
  });
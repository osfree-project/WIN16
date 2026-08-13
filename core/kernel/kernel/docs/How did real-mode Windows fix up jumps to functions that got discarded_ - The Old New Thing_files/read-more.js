// Dynamically move read next to bottom for responsive
document.addEventListener('DOMContentLoaded', function () {
  var readNext = document.querySelector('.read-next');
  var newContainer = document.querySelector('.read-next-mobile');
  var desktopContainer = document.querySelector('.read-next-desktop');
  var ctaContainer = document.querySelector('.cta-section-post');
  function moveReadNextOnSmallScreens() {
    // Check the window width
    if (window.innerWidth > 1084 && readNext && newContainer) {
      // Move the .read-next div to .read-next-mobile if screen is small
      newContainer.appendChild(readNext);
      if(ctaContainer){
      newContainer.after(ctaContainer); 
    }
    } else {
      // Move the .read-next div back to .read-next-desktop if screen is large
      if (window.innerWidth <= 1084 && desktopContainer && desktopContainer) {
        desktopContainer.appendChild(readNext);      
        if(ctaContainer){
          desktopContainer.after(ctaContainer); 
        }
      }
    }
  }

  // Initial move based on screen width
  moveReadNextOnSmallScreens();

  // Listen for window resize events
  window.addEventListener('resize', function () {
    // Recheck and move when window size changes
    moveReadNextOnSmallScreens();
  });
});
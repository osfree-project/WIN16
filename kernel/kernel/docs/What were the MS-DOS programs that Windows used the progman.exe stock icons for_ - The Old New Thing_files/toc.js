// Builds the Table of content and add scroll based on height
document.addEventListener("DOMContentLoaded", function () {
  // Select the main content area where h2 headings are located
  const mainContent = document.querySelector(".entry-content");
  const tocSidebar = document.querySelector("#toc-sidebar");
  const tocList = document.querySelector("#toc");
  const tocTitle = document.querySelector(".toc-title");
  // Find all h2 headings in the main content
  const headings = mainContent.querySelectorAll("h2, h3");

  // Function to generate the table of contents
  function generateTOC() {
    // Clear the existing table of contents
    tocList.innerHTML = "";

    // Check if there are h2 elements
    if (headings.length === 0 || headings.length < 3) {
      tocSidebar.style.display = "none";
    } else {
      tocSidebar.style.display = "block";
      // Set the title of the table of contents
      tocTitle.innerHTML = "Table of contents";
    }

    // Create a Set to store unique heading texts
    const seenHeadings = new Set();

    // Loop through each h2 and h3 heading and generate the table of contents
    headings.forEach(function (heading) {
      // Get the trimmed text content of the heading
      const headingText = heading.textContent.trim();

      // Check if the heading has text content and is not a duplicate
      if (headingText === "" || seenHeadings.has(headingText)) {
        return; // Skip empty or duplicate headings
      }

      // Add the heading text to the Set to track it as "seen"
      seenHeadings.add(headingText);

      // Generate an ID based on the heading's text content
      const id = headingText
        .toLowerCase()
        .replace(/\s+/g, "-")
        .replace(/,/g, '')
        .replace(/!/g, '')
        .replace(/\?/g, ''); // add index at the end to ensure unique id

      // Set the generated ID as the heading's id attribute
      heading.id = id;

      // Create a list item for each heading
      const listItem = document.createElement("li");
      listItem.classList.add("toc-item");

      // Create an anchor element to link to the heading
      const anchor = document.createElement("a");
      anchor.textContent = headingText;
      anchor.href = "#" + id;
      anchor.classList.add("evo-dropdown-item");

      // Add additional attributes for tracking
      anchor.setAttribute("data-bi-area", "sidebar_right_table_of_contents");
      anchor.setAttribute("data-bi-id", "post_page_sidebar_right_table_of_contents");
      anchor.setAttribute("data-bi-name", headingText);

      // Append the anchor to the list item
      listItem.appendChild(anchor);

      // Append the list item to the table of contents
      tocList.appendChild(listItem);
    });

    // After TOC generation, adjust sidebar if needed
    window.addEventListener('resize', checkOverflow);
  }

  // Add scroll on overflow
  function checkOverflow() {
    const postSidebar = document.querySelector(".post-sidebar");
    const getSideBarHeight = document.getElementById('get-height');

    if (!postSidebar || !getSideBarHeight) {
      console.log("Elements not found.");
      return;
    }
    const containerHeight = postSidebar.clientHeight;
    const contentHeight = getSideBarHeight.scrollHeight + 20;

    if (contentHeight > containerHeight) {
      postSidebar.style.overflowY = 'scroll';
    } else {
      postSidebar.style.removeProperty('overflow-y');
    }
  }

  generateTOC();

  // Call the function on page load
  window.addEventListener('load', function () {
    checkOverflow();
  });

  let clickEventOccurred = false; // Flag to indicate whether a click event has occurred
  let delayedExecutionTimer; // Timer for delayed execution of updateTOC function

  function updateTOC() {
    if (clickEventOccurred) {
      // If a click event occurred, delay the execution of updateTOC by 50 milliseconds
      clearTimeout(delayedExecutionTimer); // Clear any existing timer
      delayedExecutionTimer = setTimeout(() => {
        clickEventOccurred = false; // Reset the flag
      }, 50);
      return; // Exit the function if a click event has occurred
    }

    // Select only h2 and h3 headings within .entry-content
    const sections = document.querySelectorAll('.entry-content h2[id], .entry-content h3[id]');
    const navLinks = document.querySelectorAll('ul#toc li a');

    // Log the sections being selected
    // console.log("Sections:", sections);

    // Find the section that is currently in view
    let activeSectionId = '';
    sections.forEach((section, index) => {
      const rect = section.getBoundingClientRect();
      const isInViewport = rect.top <= window.innerHeight && rect.bottom >= 0;
      const isAboveViewport = rect.top < 0 && rect.bottom < window.innerHeight;

      // Log the rect and viewport status
      // console.log(`Section ${section.getAttribute('id')}:`, rect, `isInViewport: ${isInViewport}`, `isAboveViewport: ${isAboveViewport}`);

      if (isInViewport || isAboveViewport) {
        activeSectionId = section.getAttribute('id');

        // Add the 'active' class to the corresponding navigation link
        const activeLink = document.querySelector(`#toc a[href="#${activeSectionId}"]`);
        if (activeLink) {
          activeLink.classList.add('active');
          // console.log(`Setting activeSectionId to: ${activeSectionId}`);
        }

        // Remove the 'active' class from all other navigation links
        navLinks.forEach(link => {
          if (link !== activeLink) {
            link.classList.remove('active');
            // console.log(`Removing 'active' class from link: ${link.getAttribute('href')}`);
          }
        });

        // Break out of the loop if the section is in the viewport or above it
        if (isInViewport || index === 0) {
          return;
        }
      }
    });

    if (sections.length > 0) {
      // Check if the first section (first h2) is below the viewport
      const firstSectionRect = sections[0].getBoundingClientRect();
      if (firstSectionRect.top > window.innerHeight) {
        navLinks[0].classList.remove('active');
        // console.log("First section is below the viewport, removing active class from first TOC link");
      }
      // Check if the last section (last h2) is above the viewport
      const lastSectionRect = sections[sections.length - 1].getBoundingClientRect();
      if (lastSectionRect.bottom < 0) {
        navLinks[sections.length - 1].classList.remove('active');
        // console.log("Last section is above the viewport, removing active class from last TOC link");
      }
    }
  }

  // Run
  window.addEventListener("scroll", updateTOC);

  // Use clickEventOccurred flag to set state
  function setupNavLinkClicks(selector) {
    const navLinks = document.querySelectorAll(selector);
    navLinks.forEach(link => {
      link.addEventListener('click', function (event) {
        // Set the flag to true to indicate a click event has occurred
        clickEventOccurred = true;
        // Remove active class from all navigation links in the list
        navLinks.forEach(navLink => {
          navLink.classList.remove('active');
        });
        // Add active class to the clicked navigation link
        link.classList.add('active');
      });
    });
  }
  // Run
  setupNavLinkClicks('ul#toc li a');

  // Create the show more
  function checkH2Count() {
    // Select the main content area where h2 headings are located
    return headings.length > 8;
  }

  // Function to handle media query change
  function handleMediaQueryChange(event) {
    if (event.matches) {
      // Screen width is greater than 1084px
      const isH2CountOver10 = checkH2Count();

      const showMoreBtn = document.getElementById("showMoreBtn");

      if (isH2CountOver10) {
        //console.log("There are more than 10 h2 elements.");
        const tocItems = document.querySelectorAll(".toc-item"); // Select all list items with class "toc-item"

        // Hide list items starting from the 9th one
        for (let i = 8; i < tocItems.length; i++) {
          tocItems[i].style.display = "none";
        }

        // Set the title of the table of contents
        showMoreBtn.classList.add('active');
        showMoreBtn.classList.add("mt-4");

        // Function to toggle the visibility of the anchor count list
        function toggleAnchorCountList() {
          for (let i = 8; i < tocItems.length; i++) {
            tocItems[i].style.display = (tocItems[i].style.display === "none") ? "flex" : "none";
          }

          // Toggle the text content of the "Show more" button
          showMoreBtn.textContent = (showMoreBtn.textContent === "Show more") ? "Show less" : "Show more";
          showMoreBtn.setAttribute("data-bi-id", (showMoreBtn.textContent === "Show more") ? "post_page_sidebar_right_table_of_contents_show_more" : "post_page_sidebar_right_table_of_contents_show_less");
          showMoreBtn.setAttribute("data-bi-name", (showMoreBtn.textContent === "Show more") ? "Show more" : "Show less");

          // Call checkOverflow after toggling visibility
          checkOverflow();
        }

        // Add click event listener to the "Show more" button
        showMoreBtn.addEventListener("click", toggleAnchorCountList);
      } else {
        // If there are 8 or fewer h2 elements, remove the "Show more" button
        if (showMoreBtn) {
          showMoreBtn.classList.remove('active');
        }
        //console.log("There are 10 or fewer h2 elements.");
      }

      function checkIfNinthTocItemIsActive() {
        const tocItems = document.querySelectorAll('#toc li.toc-item a.evo-dropdown-item');

        if (tocItems.length >= 9) {
          let ninthItemActive = false;

          for (let i = 8; i < tocItems.length; i++) {
            const currentItem = tocItems[i];

            if (currentItem.classList.contains('active')) {
              ninthItemActive = true;
              break;
            }
          }

          if (ninthItemActive) {
            const allTocItems = document.querySelectorAll('.toc-item');
            allTocItems.forEach(item => {
              item.style.display = 'flex';
            });
            showMoreBtn.innerHTML = "Show Less";
          }
        }
      }

      checkIfNinthTocItemIsActive();
      window.addEventListener('scroll', checkIfNinthTocItemIsActive);
    }
  }


  // Create a media query list
  const mediaQuery = window.matchMedia('(min-width: 1084px)');

  // Add listener for media query changes
  mediaQuery.addEventListener('resize', handleMediaQueryChange);

  // Initial check
  handleMediaQueryChange(mediaQuery);
});

// Table of contents mobile
document.addEventListener("DOMContentLoaded", function () {

  // Use position fixed because stick causes scroll on toggle
function updateTocFixed() {
  const postSidebar = document.querySelector(".post-sidebar");

  if (!postSidebar) {
      return; // Exit if the sidebar element is not found
  }

  // Detect pinch-to-zoom and adjust positioning
  let isZoomedIn = false;
  if ('visualViewport' in window) {
      isZoomedIn = window.visualViewport.scale > 1; // Adjust if you need a different zoom threshold
  }

  if (window.innerWidth < 1084 && !isZoomedIn) { // Only apply if not zoomed in
      const entryContent = document.querySelector("#single-wrapper.container-three-column-post");
      if (!entryContent) {
          return; // Exit if the entry content element is not found
      }
      const entryContentRect = entryContent.getBoundingClientRect();
      const entryContentTop = entryContentRect.top; // Adjust threshold as needed

      if (entryContentTop < 5) {
          postSidebar.style.position = "fixed";
          postSidebar.classList.add("mobile-toc-fixed");
      } else if (entryContentTop > 5) { // Modified to prevent potential jitter
          postSidebar.style.position = "relative"; // Reset to original position
          postSidebar.classList.remove("mobile-toc-fixed");
      }

      // Ensure that all .toc-item elements are visible on mobile
      const tocItems = document.querySelectorAll('.toc-item');
      tocItems.forEach(item => {
          item.style.display = 'flex';
      });
      document.getElementById('showMoreBtn').classList.remove('active'); // Removes show more mobile
  } else {
      postSidebar.style.position = ""; // Ensure position is cleared when above breakpoint or zoomed in
  }
}

// Call updateTocFixed initially and add scroll and resize event listeners
updateTocFixed();
window.addEventListener('scroll', updateTocFixed);
window.addEventListener('resize', updateTocFixed);

// Listen for zoom changes
if ('visualViewport' in window) {
  window.visualViewport.addEventListener('resize', updateTocFixed);
}

});

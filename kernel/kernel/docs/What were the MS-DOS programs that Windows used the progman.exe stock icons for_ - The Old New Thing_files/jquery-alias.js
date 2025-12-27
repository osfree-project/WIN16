jQuery(document).ready(function() {
    check_jQuery();
});   
function check_jQuery(){
    if (typeof $ === 'undefined') {
      //  console.log('The $ symbol is not defined.');
        window.$ = jQuery;
    } 
}
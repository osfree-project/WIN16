jQuery( document ).ready( function( $ ) {
	$( '.mpp-gutenberg-tab' ).on( 'click', function( e ) {
		$('.mpp-author-tabs li').removeClass('active');
		$(this).addClass('active');
		var $tabs = $('.mpp-tab').removeClass('mpp-tab-active');
		var new_tab = $(this).data('tab');
		$('.' + new_tab).addClass('mpp-tab-active');
	} );
} );
(function($){
  Drupal.behaviors.uc_GoCardless_settings = function(context) {
    $('#edit-uc-gocardless-sandbox').change(function() {
      $sandbox = $('fieldset.gocardless-sandbox');
      if ($(this).attr('checked')) {
        $sandbox.removeClass('collapsed').children('.fieldset-wrapper').css('display', 'block');
      }
      else {
        $sandbox.addClass('collapsed').children('.fieldset-wrapper').css('display', 'none');
      }

    });
  }
})(jQuery);

Drupal.Views = {};

/**
 * jQuery UI tabs, Views integration component
 */

Drupal.behaviors.viewsTabs = function (context) {
  if ($.ui && $.ui.tabs) {
    $('#views-tabset:not(.views-processed)').addClass('views-processed').tabs({
      selectedClass: 'active'
    });
  }

  $('a.views-remove-link')
    .addClass('views-processed')
    .click(function() {
      var id = $(this).attr('id').replace('views-remove-link-', '');
      $('#views-row-' + id).hide();
      $('#views-removed-' + id).attr('checked', true);
      return false;
    });
}

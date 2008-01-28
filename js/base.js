Drupal.Views = {};

/**
 * jQuery UI tabs, Views integration component
 */

Drupal.behaviors.viewsTabs = function (context) {
  $('.views-tabset:not(.views-processed)').addClass('views-processed').tabs({
    selectedClass: 'active'
  });
}

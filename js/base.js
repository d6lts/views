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

/**
 * Helper function to parse a querystring.
 */
Drupal.Views.parseQueryString = function (query) {
  var args = {};
  var pos = query.indexOf('?');
  if (pos != -1) {
    query = query.substring(pos + 1);
  }
  var pairs = query.split('&');
  for(var i in pairs) {
    var pair = pairs[i].split('=');
    // Ignore the 'q' path argument, if present.
    if (pair[0] != 'q') {
      args[pair[0]] = unescape(pair[1].replace(/\+/g, ' '));
    }
  }
  return args;
};

/**
 * Helper function to return the last portion of a class.
 */
Drupal.Views.getClassSuffix = function (elt, prefix) {
  var className = elt.className.split(' ');
  for (var i in className) {
    if (className[i].indexOf(prefix) == 0) {
      return className[i].substring(prefix.length);
    }
  }
  return false;
};
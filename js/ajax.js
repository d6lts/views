// $Id$
/**
* @file ajax.inc
*
* Handles AJAX submission and response in Views UI.
*/

Drupal.Views.Ajax = {};

/**
 * Handles the simple process of setting the ajax form area with new data.
 */
Drupal.Views.Ajax.setForm = function(title, output) {
  $(Drupal.settings.views.ajax.title).html(title);
  $(Drupal.settings.views.ajax.id).html(output);
}

/**
 * An ajax responder that accepts a packet of JSON data and acts appropriately.
 *
 * The following fields control behavior.
 * - 'display': Display the associated data in the form area; bind the new
 *   form to 'url' if appropriate. The 'title' field may also be used.
 * - 'add': This is a keyed array of HTML output to add via append. The key is
 *   the id to append via $(key).append(value)
 * - 'replace': This is a keyed array of HTML output to add via replace. The key is
 *   the id to append via $(key).html(value)
 * 
 */
Drupal.Views.Ajax.ajaxResponse = function(data) {
  if (data.debug) {
    alert(data.debug);
  }

  // See if we have any settings to extend. Do this first so that behaviors
  // can access the new settings easily.

  if (data.js) {
    $.extend(Drupal.settings, data.js);
  }

  // Check the 'display' for data.
  if (data.display) {
    Drupal.Views.Ajax.setForm(data.title, data.display);

    // if a URL was supplied, bind the form to it.
    if (data.url) {
      var ajax_area = Drupal.settings.views.ajax.id;
      var ajax_title = Drupal.settings.views.ajax.title;
    
      // Bind a click to the button to set the value for the button.
      $('input[type=submit]', ajax_area).unbind('click');
      $('input[type=submit]', ajax_area).click(function() {
        $('form', ajax_area).append('<input type="hidden" name="' 
          + $(this).attr('name') + '" value="' + $(this).val() + '">');
      });

      // Bind forms to ajax submit.
      $('form', ajax_area).unbind('submit'); // be safe here.
      $('form', ajax_area).submit(function() {
        $(this).ajaxSubmit({
          url: data.url,
          data: '',
          type: 'POST',
          success: Drupal.Views.Ajax.ajaxResponse,
          error: function() { alert("An error occurred."); },
          dataType: 'json'
        });
        return false;
      });
    }

    Drupal.attachBehaviors(ajax_area);
  }
  else {
    // If no display, reset the form.
    Drupal.Views.Ajax.setForm('', Drupal.settings.views.ajax.defaultForm);
  } 

  // Go through the 'add' array and add any new content we're instructed to add.
  if (data.add) {
    for (id in data.add) {
      var newContent = $(id).append(data.add[id]);
      Drupal.attachBehaviors(newContent);
    }
  }

  // Go through the 'replace' array and replace any content we're instructed to.
  if (data.replace) {
    for (id in data.replace) {
      $(id).html(data.replace[id]);
      Drupal.attachBehaviors(id);
    }
  }

  // Go through and add any requested tabs
  if (data.tab) {
    for (id in data.tab) {
      console.log(data.tab[id]['title']);
      $('#views-tabset').addTab(id, data.tab[id]['title'], 0);
      $(id).html(data.tab[id]['body']);
      $(id).addClass('views-tab');
      Drupal.attachBehaviors(id);

      // This is kind of annoying, but we have to actually to find where the new
      // tab is.
      var instance = $.ui.tabs.instances[$('#views-tabset').get(0).UI_TABS_UUID];
      $('#views-tabset').clickTab(instance.$tabs.length);
    }
  }

}

Drupal.behaviors.ViewsAjaxLinks = function() {
  // Make specified links ajaxy.
  $('a.views-ajax-link:not(.views-processed)').addClass('views-processed').click(function() {
    // Translate the href on the link to the ajax href. That way this degrades
    // into a nice, normal link.
    var url = $(this).attr('href');
    url = url.replace('nojs', 'ajax');

    // Turn on the hilite to indicate this is in use.
    $(this).addClass('hilite');

    $.ajax({
      type: "GET",
      url: url,
      data: '',
      success: Drupal.Views.Ajax.ajaxResponse,
      error: function() { alert("An error occurred."); },
      dataType: 'json'
    });
    
    return false;
  });  

  $('form.views-ajax-form:not(.views-processed)').addClass('views-processed').submit(function() {
    // Translate the href on the link to the ajax href. That way this degrades
    // into a nice, normal link.
    var url = $(this).attr('action');
    url = url.replace('nojs', 'ajax');

    $(this).ajaxSubmit({
      url: url,
      data: '',
      type: 'POST',
      success: Drupal.Views.Ajax.ajaxResponse,
      error: function() { alert("An error occurred."); },
      dataType: 'json'
    });

    return false;   
  });

}

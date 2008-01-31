<?php

class views_tabset {
  var $tabs = array();
  var $extra = '';
  var $selected = NULL;

  function add($name, $title = '', $body = '') {
    if (is_object($name) && is_subclass_of($name, 'views_tab')) {
      $this->add_tab($name);
    }
    elseif (is_array($name)) {
      foreach ($name as $real_tab) {
        $this->add($real_tab);
      }
    }
    else {
      $this->add_tab(new views_tab($name, $title, $body));
    }
  }

  function add_tab($tab) {
    $this->tabs[$tab->name] = $tab;
  }

  function set($name, $title, $body = NULL) {
    if (empty($this->tabs[$name])) {
      return $this->add($name, $title, $body);
    }
    $this->tabs[$name]->title = $title;
    if (isset($body)) {
      $this->tabs[$name]->body = $body;
    }
  }

  function set_body($name, $body) {
    if (empty($this->tabs[$name])) {
      return $this->add($name, '', $body);
    }
    $this->tabs[$name]->body = $body;
  }

  function add_extra($text) {
    $this->extra .= $text;
  }

  function remove($tab) {
    if (is_string($tab)) {
      unset($this->tabs[$tab]);
    }
    else {
      unset($this->tabs[$tab->name]);
    }
  }

  function set_selected($name) {
    $this->selected = $name;
  }

  function render() {
    views_add_js('tabs');
    views_add_css('tabs');

    if (empty($this->selected)) {
      $keys = array_keys($this->tabs);
      $this->selected = array_shift($keys);
    }

    drupal_alter('views_tabset', $this);
    return theme('views_tabset', $this->tabs, $this->extra, $this->selected);
  }
}

class views_tab {
  var $title;
  var $body;
  var $name;

  function views_tab($name, $title, $body = NULL) {
    $this->name = $name;
    $this->title = $title;
    $this->body = $body;
  }

  function render() {
    return theme('views_tab', $this->body);
  }
}

function theme_views_tabset($tabs, $extra = NULL, $selected = NULL) {
  $link_output = "<div class=\"views-tabs\"><ul id=\"views-tabset\">\n";
  $tab_output = "<div class=\"views-tab-area\">\n";

  foreach ($tabs as $name => $tab) {
    $link_output .= '<li' . ($name == $selected ? ' class="active"': '') . '><a href="#views-tab-' . $tab->name . '" id="views-tab-title-' . $tab->name . '">' . check_plain($tab->title) . '</a></li>' . "\n";
    $tab_output .= '<div id="views-tab-' . $tab->name . '" class="views-tab">' . $tab->render() . "</div>\n";
  }
  $link_output .= "</ul>\n";

  if ($extra) {
    $link_output .= "<div class=\"extra\">$extra</div>\n";
  }

  $link_output .= "</div>\n";
  $tab_output .= "</div>\n";
  return '<div class="views-tabset clear-block">' . $link_output . $tab_output . '</div>';
}

function theme_views_tab($body) {
  return $body;
}

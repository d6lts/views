<?php
// $Id$

/**
 * Field handler to provide simple renderer that allows linking to a node.
 */
class views_handler_field_search_score extends views_handler_field_numeric {
  function query() {
    // Check to see if the search filter added 'score' to the table.
    // Our filter stores it as $handler->search_score -- and we also
    // need to check its relationship to make sure that we're using the same
    // one or obviously this won't work.
    foreach ($this->view->filter as $handler) {
      if (isset($handler->search_score) && $handler->relationship == $this->relationship) {
        $this->field_alias = $handler->search_score;
        $this->table_alias = $handler->table_alias;
        return;
      }
    }

    // Hide this field if no search filter is in place.
    $this->options['exclude'] = TRUE;
  }

  function click_sort($order) {
    if (isset($this->table_alias)) {
       $this->query->add_orderby(NULL, NULL, $order, $this->field_alias);
    }
  }

  function render($values) {
    // Only render if we exist.
    if (isset($this->table_alias)) {
      return parent::render($values);
    }
  }

}

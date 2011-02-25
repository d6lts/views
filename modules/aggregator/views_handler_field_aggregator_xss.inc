<?php
/**
 * @file
 *   Filters htmls tags from item.
 */

class views_handler_field_aggregator_xss extends views_handler_field {
  /**
   * Override the function used to sanitize the value before output
   * from the render function.
   */
  function sanitize_value($value) {
    return aggregator_filter_xss($value);
  }
}

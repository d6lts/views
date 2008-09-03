<?php
// $Id$
/**
 * Abstract argument handler for dates.
 *
 * Adds an option to set a default argument based on the current date.
 *
 * @param $arg_format
 *   The format string to use on the current time when
 *   creating a default date argument.
 *
 * Definitions terms:
 * - many to one: If true, the "many to one" helper will be used.
 * - invalid input: A string to give to the user for obviously invalid input.
 *                  This is deprecated in favor of argument validators.
 * @see views_many_to_one_helper
 *
 * @ingroup views_argument_handlers
 */
class views_handler_argument_date extends views_handler_argument_formula {
  var $option_name = 'default_argument_date';
  var $arg_format = 'Y-m-d';

  /**
   * Add an option to set the default value to the current date.
   */
  function default_argument_form(&$form, &$form_state) {
    parent::default_argument_form($form, $form_state);
    $form['default_argument_type']['#options'] += array('date' => t('Current date'));
  }

  /**
   * Set the empty argument value to the current date,
   * formatted appropriately for this argument.
   */
  function get_default_argument($raw = FALSE) {
    if (!$raw && $this->options['default_argument_type'] == 'date') {
      return date($this->arg_format, time());
    }
    else {
      return parent::get_default_argument($raw);
    }
  }
}

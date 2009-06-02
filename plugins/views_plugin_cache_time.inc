<?php
// $Id$

/**
 * Simple caching of query results for Views displays.
 */
class views_plugin_cache_time extends views_plugin_cache {
  /**
   * Initialize the plugin.
   *
   * @param $view
   *   The view object.
   * @param $display
   *   The display handler.
   */
  function init(&$view, &$display) {
    $this->view = &$view;
    $this->display = &$display;
    $this->options = array();

    if (is_object($display->handler)) {
    // Note: The below is read only.
      $this->options = $display->handler->get_option('cache');
    }
  }

  /**
   * Retrieve the default options when this is a new access
   * control plugin
   */
  function option_defaults(&$options) {
    $options['results_lifespan'] = 3600;
    $options['output_lifespan'] = 3600;
  }

  /**
   * Provide the default form for setting options.
   */
  function options_form(&$form, &$form_state) {
    $options = array(60, 300, 1800, 3600, 21600, 518400);
    $options = drupal_map_assoc($options, 'format_interval');
    $options = array(-1 => t('Never cache')) + $options;
                  
    $form['results_lifespan'] = array(
      '#type' => 'select',
      '#title' => t('Query results'),
      '#description' => t('The length of time raw query results should be cached.'),
      '#options' => $options,
      '#default_value' => $this->options['results_lifespan'],
    );
    $form['output_lifespan'] = array(
      '#type' => 'select',
      '#title' => t('Rendered output'),
      '#description' => t('The length of time rendered HTML output should be cached.'),
      '#options' => $options,
      '#default_value' => $this->options['output_lifespan'],
    );
  }

  /**
   * Provide the default form form for validating options
   */
  function options_validate(&$form, &$form_state) { }

  /**
   * Provide the default form form for submitting options
   */
  function options_submit(&$form, &$form_state) { }

  /**
   * Return a string to display as the clickable title for the
   * access control.
   */
  function summary_title() {
    return format_interval($this->options['results_lifespan'], 1) .'/'. format_interval($this->options['output_lifespan'], 1);
  }

  /**
   * Save data to the cache.
   */
  function cache_set($type) {
    if ($lifespan = $this->options[$type .'_lifespan']) {
      $cutoff = time() - $lifespan;
    }
    else {
      return FALSE;
    }

    switch ($type) {
      case 'query':
        // Not supported currently, but this is certainly where we'd put it.
        break;
      case 'results':
        $data = array(
          'result' => $this->view->result,
          'total_rows' => $this->view->total_rows,
          'pager' => $this->view->pager,
        );
        cache_set($this->_results_key(), $data, 'cache_views_data');
        break;
      case 'output':
        $this->gather_headers();
        $data = array(
          'head' => $this->head,
          'css' => $this->css,
          'js' => $this->js,
          'output' => $this->view->display_handler->output,
        );
        cache_set($this->_output_key(), $data, 'cache_views_data');
        break;
    }
  }

  /**
   * Retrieve data from the cache.
   */
  function cache_get($type) {
    if ($lifespan = $this->options[$type .'_lifespan']) {
      $cutoff = time() - $lifespan;
    }
    else {
      return FALSE;
    }

    switch ($type) {
      case 'query':
        // Not supported currently, but this is certainly where we'd put it.
        return FALSE;
      case 'results':
        // Values to set: $view->result, $view->total_rows, $view->execute_time,
        // $view->pager['current_page'].
        if ($cache = cache_get($this->_results_key(), 'cache_views_data')) {
          if ($cache->created > $cutoff) {
            $this->view->result = $cache->data['result'];
            $this->view->total_rows = $cache->data['total_rows'];
            $this->view->pager = $cache->data['pager'];
            $this->view->execute_time = 0;
            return TRUE;
          }
        }
        return FALSE;
      case 'output':
        if ($cache = cache_get($this->_output_key(), 'cache_views_data')) {
          if ($cache->created > $cutoff) {
            $this->view->display_handler->output = $cache->data['output'];
            $this->head = $cache->data['head'];
            $this->css = $cache->data['css'];
            $this->js = $cache->data['js'];
            $this->restore_headers();
            return TRUE;
          }
        }
        return FALSE;
    }
  }
  
  /**
   * Clear out cached data for a view.
   *
   * We're jsut going to nuke anything related to the view, regardless of display,
   * to be sure that we catch everything. Maybe that's a bad idea.
   */
  function cache_flush() {
    cache_clear_all($this->view->name .':', 'cache_views_data', TRUE);
  }
  
  function _results_key() {
    $key_data = array(
      'build_info' => $this->view->build_info,
      'exposed_info' => $this->view->exposed_info,
      'page' => $_GET['page'],
    );
    return $this->view->name .':'. $this->display->id .':results:'. md5(serialize($key_data));
  }
  
  function _output_key() {
    $key_data = array(
      'result' => $this->view->result,
    );
    return $this->view->name .':'. $this->display->id .':output:'. md5(serialize($key_data));
  }
}

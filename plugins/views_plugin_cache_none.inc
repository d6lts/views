<?php
// $Id$

/**
 * Caching plugin that provides no caching at all.
 */
class views_plugin_cache_none extends views_plugin_cache {
  function summary_title() {
    return t('None');
  }
}

<?php

/**
 * @file
 * flipclock7.features.field_base.inc
 */

/**
 * Implements hook_field_default_field_bases().
 */
function flipclock7_field_default_field_bases() {
  $field_bases = array();

  // Exported field_base: 'flipclock7'.
  $field_bases['flipclock7'] = array(
    'active' => 1,
    'cardinality' => 1,
    'deleted' => 0,
    'entity_types' => array(),
    'field_name' => 'flipclock7',
    'global_block_settings' => 2,
    'indexes' => array(),
    'locked' => 0,
    'module' => 'flipclock7',
    'settings' => array(
      'cache_count' => 4,
      'cache_enabled' => 0,
      'granularity' => array(
        'day' => 'day',
        'hour' => 'hour',
        'minute' => 'minute',
        'month' => 'month',
        'second' => 0,
        'year' => 'year',
      ),
      'heading' => NULL,
      'timezone_db' => 'UTC',
      'todate' => '',
      'tz_handling' => 'site',
      'visibility' => 0,
    ),
    'translatable' => 0,
    'type' => 'flipclock7',
  );

  return $field_bases;
}

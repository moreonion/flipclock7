<?php

/**
 * @file
 * Field API functions based on the date module.
 */

module_load_include('inc', 'date', 'date.field');
module_load_include('inc', 'date', 'date_elements');
module_load_include('module', 'date', 'date');

/**
 * Helper function to remove the flipclock7_ prefix from a string.
 */
function _flipclock7_remove_prefix(&$str) {
  if (substr($str, 0, 11) == 'flipclock7_') {
    $str = substr($str, 11);
  }
}

/**
 * Implements hook_field_load().
 */
function flipclock7_field_load($entity_type, $entities, $field, $instances, $langcode, &$items, $age) {
  date_field_load($entity_type, $entities, $field, $instances, $langcode, $items, $age);
}

/**
 * Implements hook_field_widget_settings_form().
 */
function flipclock7_field_widget_settings_form($field, $instance) {
  module_load_include('inc', 'date', 'date_admin');
  return _date_field_widget_settings_form($field, $instance);
}

/**
 * Implements hook_field_settings_form().
 */
function flipclock7_field_settings_form($field, $instance, $has_data) {
  $form = date_field_settings_form($field, $instance, $has_data);

  // Fixate tz_handling to the timezone of the site.
  $form['tz_handling']['#default_value'] = 'site';
  $form['tz_handling']['#access'] = FALSE;

  // Disable setting of enddate.
  $form['enddate_get']['#default_value'] = 0;
  $form['enddate_get']['#access'] = FALSE;
  $form['enddate_required']['#default_value'] = 0;
  $form['enddate_required']['#access'] = FALSE;

  return $form;
}

/**
 * Implements hook_field_instance_settings_form().
 *
 * Wrapper functions for date administration, included only when processing
 * field settings.
 */
function flipclock7_field_instance_settings_form($field, $instance) {
  module_load_include('inc', 'date', 'date_admin');
  return _date_field_instance_settings_form($field, $instance);
}

/**
 * Implements hook_form_FORM_ID_alter() for field_ui_field_edit_form().
 */
function flipclock7_form_field_ui_field_edit_form_alter(&$form, &$form_state, $form_id) {
  if ($form['#field']['type'] === 'flipclock7') {
    $form['#field']['type'] = 'datestamp';
    date_form_field_ui_field_edit_form_alter($form, $form_state, $form_id);
    $form['#field']['type'] = 'flipclock7';
  }
}

/**
 * Implements hook_field_widget_form().
 */
function flipclock7_field_widget_form(&$form, &$form_state, &$field, $instance, $langcode, $items, $delta, $element) {
  $item = ($items[$delta] ?? []) + [
    'visibility' => NULL,
    'heading' => '',
  ];
  $item += $field['settings'];

  $visibility_id = drupal_html_id('flipclock7-visibility');
  $element['visibility'] = [
    '#type' => 'checkbox',
    '#title' => t('Display a countdown'),
    '#default_value' => $item['visibility'],
    '#weight' => -10,
    '#id' => $visibility_id,
  ];
  $visibility_state['#states']['visible']["#$visibility_id"]['checked'] = TRUE;

  $dt_field = ['type' => 'datestamp'] + $field;
  $dt_instance = $instance;
  // Strip flipclock7_ from the widget_type to get a valid
  // date module widget type.
  _flipclock7_remove_prefix($dt_instance['widget']['type']);
  $time_items[$delta] = isset($item['value']) ? ['value' => $item['value']] : [];
  $element['end'] = [
    // The title is set again in flipclock7_date_combo_process_alter() as
    // it will be overidden by the date module.
    '#title' => t('Countdown end time'),
    '#description' => t('Point in time where countdown’s value reaches zero.'),
    '#flipclock7_container' => TRUE,
    '#element_validate' => ['flipclock7_datetime_validate'],
  ] + date_field_widget_form($form, $form_state, $dt_field, $dt_instance, $langcode, $time_items, $delta, [
    '#field_name' => $element['#field_name'],
    '#delta' => $element['#delta'],
    '#bundle' => $element['#bundle'],
    '#entity_type' => $element['#entity_type'],
    '#field_parents' => $element['#field_parents'],
    '#language' => $element['#language'],
  ]) + $visibility_state;

  $element['heading'] = [
    '#type' => 'textfield',
    '#title' => t('Heading'),
    '#maxlength' => 90,
    '#default_value' => $item['heading'],
  ] + $visibility_state;

  $element += [
    '#type' => 'fieldset',
    '#title' => $instance['label'],
    '#description' => $instance['description'],
    '#tree' => TRUE,
    '#element_validate' => ['flipclock7_validate'],
  ];
  $element['#attributes']['class'][] = 'field-group-heading';
  $element['#attached']['css'][drupal_get_path('module', 'flipclock7') . '/css/flipclock7_admin.css']['type'] = 'file';
  return $element;
}

/**
 * Implements hook_date_combo_process_alter().
 *
 * Render our datetime elements as containers.
 */
function flipclock7_date_combo_process_alter(&$element, $form_state, $context) {
  if (!empty($element['#flipclock7_container'])) {
    $element['#theme_wrappers'] = ['container'];
    $element['value'] += [
      '#title' => t('Countdown end time'),
    ];
  }
}

/**
 * Element validate callback: Validate the nested date element.
 *
 * @see flipclock7_field_widget_form()
 */
function flipclock7_datetime_validate($element, &$form_state) {
  module_load_include('inc', 'date', 'date_elements');
  $element['value']['#field']['type'] = 'datestamp';
  $parents = _field_form_state_parents($element['#field_parents'], $element['#field_name'], $element['#language']);
  $field_state = &drupal_array_get_nested_value($form_state, $parents);
  $backup = $field_state;
  _flipclock7_remove_prefix($field_state['instance']['widget']['type']);
  date_combo_validate($element, $form_state);
  $field_state = $backup;
  $element['value']['#field']['type'] = 'flipclock7';
}

/**
 * Element validate callback: Validate and update a flipclock7 element.
 *
 * Don't try this if there were errors before reaching this point.
 *
 * @see flipclock7_field_widget_form()
 */
function flipclock7_validate($element, &$form_state) {
  $values = &drupal_array_get_nested_value($form_state['values'], $element['#parents']);
  if ($values['visibility'] === 1 && trim($values['end']['value']) == '') {
    form_error($element['end']['value'], t('You need to enter a date if the countdown timer is set to visible.'));
  }
  $values['value'] = $values['end']['value'];
}

/**
 * Implements hook_field_widget_properties_alter().
 *
 * Alters the widget properties of a field instance before it gets displayed.
 * Used here to flag new entities so we can later tell if they need default
 * values.
 */
function flipclock7_field_widget_properties_alter(&$widget, $context) {
  $orig_type = $widget['type'];
  _flipclock7_remove_prefix($widget['type']);
  $widget['type'] = $orig_type;
}

/**
 * Implements hook_field_widget_info().
 */
function flipclock7_field_widget_info() {
  $info = date_field_widget_info();

  $info['date_text']['field types'] = array('flipclock7');
  $info['flipclock7_date_text'] = $info['date_text'];
  unset($info['date_text']);

  $info['date_select']['field types'] = array('flipclock7');
  $info['flipclock7_date_select'] = $info['date_select'];
  unset($info['date_select']);

  if (module_exists('date_popup')) {
    $info['date_popup']['field types'] = array('flipclock7');
    $info['flipclock7_date_popup'] = $info['date_popup'];
    unset($info['date_popup']);
  }

  return $info;
}

/**
 * Implements hook_field_is_empty().
 */
function flipclock7_field_is_empty($item, $field) {
  date_field_is_empty($item, $field);
}

/**
 * Implements hook_field_formatter_view().
 */
function flipclock7_field_formatter_view($entity_type, $entity, $field, $instance, $langcode, $items, $display) {
  $element = array();
  $font_size = $display['settings']['font_size'];

  switch ($display['type']) {
    case 'flipclock7_default':
      foreach ($items as $delta => $item) {
        if ($item['visibility'] === '1') {
          $element[$delta] = flipclock7_display_timer($item, $entity->language, $font_size, FALSE);
        }
      }
      break;
  }

  return $element;
}

/**
 * Creates the jquery countdown timer.
 */
function flipclock7_display_timer($item, $language, $font_size) {
  $field_id = drupal_html_id("flipclock7-countdown");
  $settings[$field_id] = [
    'time' => $item['value'],
    'language' => $language,
  ];

  $element['#attached']['libraries_load'][] = ['FlipClock'];
  $element['#attached']['js'][drupal_get_path('module', 'flipclock7') . '/flipclock7.js'] = [
    'type' => 'file',
    'scope' => 'footer',
    'weight' => 5,
  ];
  $element['#attached']['js'][] = array(
    'data' => array('flipclock7' => $settings),
    'type' => 'setting',
  );

  $element['#theme'] = 'flipclock7_timer';
  $element['#timer_id'] = $field_id;
  $element['#heading'] = $item['heading'];

  return $element;
}

/**
 * Implements hook_field_formatter_info().
 */
function flipclock7_field_formatter_info() {
  return array(
    'flipclock7_default' => array(
      'label' => t('FlipClock Timer'),
      'field types' => array('flipclock7'),
      'multiple values' => FIELD_BEHAVIOR_DEFAULT,
      'settings' => array(
        'font_size' => '4',
      ),
    ),
  );
}

/**
 * Implements hook_field_info().
 */
function flipclock7_field_info() {
  $granularity = array('year', 'month', 'day', 'hour', 'minute');
  $settings = array(
    'settings' => array(
      'todate' => '',
      'granularity' => drupal_map_assoc($granularity),
      'tz_handling' => 'site',
      'timezone_db' => 'UTC',
      'visibility' => 0,
      'heading' => NULL,
    ),
    'instance_settings' => array(
      'default_value' => 'now',
      'default_value_code' => '',
      'default_value2' => 'same',
      'default_value_code2' => '',
    ),
    // Integrate with the Entity Metadata module.
    'property_type' => 'date',
    'property_callbacks' => array('date_entity_metadata_property_info_alter'),
  );

  $return = array(
    'flipclock7' => array(
      'label' => t('FlipClock Timer Field'),
      'description' => t('FlipCllock Timer Field.'),
      'default_widget' => 'flipclock7_widget',
      'default_formatter' => 'flipclock7_default',
      'default_token_formatter' => 'date_plain',
    ) + $settings,
  );

  return $return;
}

<?php

/**
 * @file
 * Field API functions based on the date module.
 */

module_load_include('inc', 'date', 'date.field');
module_load_include('inc', 'date', 'date_elements');
module_load_include('module', 'date', 'date');

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
  $field['type'] = 'datestamp';
  $element = date_field_widget_form($form, $form_state, $field, $instance, $langcode, $items, $delta, $element);
  $field['type'] = 'flipclock7';

  $item = $items[$delta] ?? [];
  $item += $field['settings'];

  // We have to hack a div around our elements because
  // the date_popup module forces the date-padding class
  // on child elements, meaning the date elements will
  // be indented by 10px.
  $element['visibility'] = array(
    '#type' => 'checkbox',
    '#title' => t('Visible'),
    '#default_value' => $item['visibility'],
    '#prefix' => '<div class="form-margin">',
    '#suffix' => '</div>',
  );

  $element['heading'] = array(
    '#type' => 'textfield',
    '#title' => t('Heading'),
    '#maxlength' => 90,
    '#attributes' => array(
      'placeholder' => t('Countdown Heading'),
    ),
    '#prefix' => '<div class="form-margin">',
    '#suffix' => '</div>',
    '#default_value' => $item['heading'],
  );

  // Strip flipclock7_ from the widget_type to get a valid
  // date module widget type.
  $element['#widget_type'] = substr($element['#widget_type'], 17);
  $element['#element_validate'] = array('flipclock7_validate');

  return $element;
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
  if ($values['visibility'] === 1 && !trim($values['value']) == '') {
    form_error($element['value'], t('You need to enter a date if the countdown timer is set to visible.'));
  }

  module_load_include('inc', 'date', 'date_elements');
  $element['value']['#field']['type'] = 'datestamp';
  $parents = _field_form_state_parents($element['#field_parents'], $element['#field_name'], $element['#language']);
  $field_state = &drupal_array_get_nested_value($form_state, $parents);
  $backup = $field_state;
  $field_state['instance']['widget']['type'] = str_replace('flipclock7_', '', $field_state['instance']['widget']['type']);
  date_combo_validate($element, $form_state);
  $field_state = $backup;
  $element['value']['#field']['type'] = 'flipclock7';
}

/**
 * Implements hook_field_widget_properties_alter().
 *
 * Alters the widget properties of a field instance before it gets displayed.
 * Used here to flag new entities so we can later tell if they need default
 * values.
 */
function flipclock7_field_widget_properties_alter(&$widget, $context) {
  if (substr($widget['type'], 0, strlen('flipclock7_')) == 'flipclock7_') {
    $orig_type = $widget['type'];
    $widget['type'] = substr($widget['type'], strlen('flipclock7_'));
    date_field_widget_properties_alter($widget, $context);
    $widget['type'] = $orig_type;
  }
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
function _flipclock7_field_formatter_view($entity_type, $entity, $field, $instance, $langcode, $items, $display) {
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

  $path = drupal_get_path('module', 'flipclock7');
  $element['#attached']['libraries_load'][] = ['FlipClock'];
  $element['#attached']['js'][] = array(
    'data' => $path . '/flipclock7.js',
    'type' => 'file',
    'scope' => 'footer',
    'weight' => 5,
  );
  $element['#attached']['js'][] = array(
    'data' => array('flipclock7' => $settings),
    'type' => 'setting',
  );
  $element['#attached']['css'][] = array(
    'data' => $path . '/css/flipclock7.css',
    'type' => 'file',
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

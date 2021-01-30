<?php
/**
 * @file
 * Farm eggs quick forms.
 */

/**
 * Implements hook_farm_quick_forms().
 */
function farm_eggs_farm_quick_forms() {
  return array(
    'eggs' => array(
      'label' => t('Eggs'),
      'permission' => 'create farm_harvest log entities',
      'form' => 'farm_eggs_form',
      'file' => 'farm_eggs.farm_quick.inc',
    ),
  );
}

// defaults the form value to zero if empty, or validates value as integer
function validate_and_default_zero($element, &$form_state) {
  $value = $element['#value'];
  if ($value == ''){
    form_set_value($element, 0, $form_state);
  }
  elseif (!is_numeric($value) || intval($value) != $value) {
    form_error($element, t('%name must be an integer.', array(
      '%name' => $element['#title'],
    )));
  }
}


/**
 * Form for adding egg harvest logs.
 */
function farm_eggs_form($form, &$form_state) {

  // Wrapper fieldset.
  $form['eggs'] = array(
    '#type' => 'fieldset',
    '#title' => t('Record an egg harvest'),
    '#description' => t('Use this form to record an egg harvest. A harvest log will be created with standard details filled in.'),

  );



  // Load active assets with the "farm_eggs" asset property.
  $asset_options = array();
  $result = db_query("SELECT a.id FROM {farm_asset_property} p LEFT JOIN {farm_asset} a ON p.id = a.id WHERE p.name = 'farm_eggs' AND a.archived = 0");
  foreach ($result as $row) {
    if (!empty($row->id)) {

      // Load the asset.
      $asset = farm_asset_load($row->id);

      // If the asset didn't load, skip it.
      if (empty($asset)) {
        continue;
      }

      // Add an option (linked to asset).
      $entity_label = entity_label('farm_asset', $asset);
      $entity_uri = entity_uri('farm_asset', $asset);
      $asset_options[$asset->id] = l($entity_label, $entity_uri['path']);
    }
  }

  // If there are asset options, add checkboxes.
  if (!empty($asset_options)) {
    $form['eggs']['assets'] = array(
      '#type' => 'radios',
      '#title' => t('Group/animal'),
      '#description' => t('Select the group/animal that these eggs came from. To add groups/animals to this list, edit their record and check the "Produces eggs" checkbox.'),
      '#options' => $asset_options,
      '#required' => TRUE,
    );

    // If there is only one option, select it by default.
    if (count($asset_options) == 1) {
      $form['eggs']['assets']['#default_value'] = array_keys($asset_options)[0];
    }
  }

  // Otherwise, show some text about adding groups/animals.
  else {
    $form['eggs']['assets'] = array(
      '#type' => 'markup',
      '#markup' => t('If you would like to associate this egg harvest log with a group/animal asset, edit their record and check the "Produces eggs" checkbox. Then you will be able to select them here.'),
      '#prefix' => '<p>',
      '#suffix' => '</p>',
    );
    //return $form;
  }

    // Quantity S.
    $form['eggs']['quantityS'] = array(
      '#type' => 'textfield',
      '#title' => t('Quantity ( class A size S )'),
      '#required' => FALSE,
      '#element_validate' => array('validate_and_default_zero'),
    );
    // Quantity M.
    $form['eggs']['quantityM'] = array(
      '#type' => 'textfield',
      '#title' => t('Quantity ( class A size M )'),
      '#required' => FALSE,
      '#element_validate' => array('validate_and_default_zero'),
    );
    // Quantity L.
    $form['eggs']['quantityL'] = array(
      '#type' => 'textfield',
      '#title' => t('Quantity ( class A size L )'),
      '#required' => FALSE,
      '#element_validate' => array('validate_and_default_zero'),
    );
    // Quantity XL.
    $form['eggs']['quantityXL'] = array(
      '#type' => 'textfield',
      '#title' => t('Quantity ( class A size XL )'),
      '#required' => FALSE,
    );
    // Quantity class B.
    $form['eggs']['quantityB'] = array(
      '#type' => 'textfield',
      '#title' => t('Quantity ( class B )'),
      '#required' => FALSE,
      '#element_validate' => array('validate_and_default_zero'),
    );
    // Quantity class C.
    $form['eggs']['quantityC'] = array(
      '#type' => 'textfield',
      '#title' => t('Quantity ( class C )'),
      '#required' => FALSE,
      '#element_validate' => array('validate_and_default_zero'),
    );

  // Submit button.
  $form['eggs']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save log'),
  );

  // Return the form.
  return $form;
}

/**
 * Submit function for eggs quick form.
 */
function farm_eggs_form_submit($form, &$form_state) {

  // The log type will be a harvest.
  $log_type = 'farm_harvest';

  // radio buttons only provide one asset.
  $assets = array(
    farm_asset_load($form_state['values']['assets'])
  );

  // Assemble an array of measurements.
  $measurements = array(
    array(
      'measure' => 'count',
      'value' => $form_state['values']['quantityS'],
      'units' => t('egg(s)'),
      'label' => 'S',
    ),
    array(
      'measure' => 'count',
      'value' => $form_state['values']['quantityM'],
      'units' => t('egg(s)'),
      'label' => 'M',
    ),
    array(
      'measure' => 'count',
      'value' => $form_state['values']['quantityL'],
      'units' => t('egg(s)'),
      'label' => 'L',
    ),
    array(
      'measure' => 'count',
      'value' => $form_state['values']['quantityXL'],
      'units' => t('egg(s)'),
      'label' => 'XL',
    ),
    array(
      'measure' => 'count',
      'value' => $form_state['values']['quantityB'],
      'units' => t('egg(s)'),
      'label' => 'ClassB',
    ),
    array(
      'measure' => 'count',
      'value' => $form_state['values']['quantityC'],
      'units' => t('egg(s)'),
      'label' => 'ClassC',
    ),

  );

  // calculate total quantity
  $totalqty = 0;
  foreach ( $measurements as $measurement) {
    $totalqty += $measurement['value'];
  }

  // Set log name.
  $log_name = t('Collected @qty egg(s) from the @flk', array('@qty' => $totalqty, '@flk' => $assets[0]->name));

  // Create a new farm quantity log.
  $log = farm_quantity_log_create($log_type, $log_name, REQUEST_TIME, TRUE, $assets, $measurements);

  // Link the log to the quick form.
  farm_quick_entity_link('farm_eggs_form', 'log', $log);
}

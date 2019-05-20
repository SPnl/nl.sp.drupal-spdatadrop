<?php

/**
 * @file
 * Unsubscribe form.
 */

/**
 * Unsubscribe form definition.
 */
function spdatadrop_unsubscribe_form($form, &$form_state, $encoded_data) {
  $items = array();

  if (!empty($form_state['unsubscribed'])) {
    // Unsubscription added.
    $items['feedback'] = array(
      '#markup' => '<p>Uw afmelding is opgeslagen. Zodra deze verwerkt is ontvangt u een e-mail ter bevestiging.</p><p>Met vriendelijke groet,<br/>SP</p>',
    );
  }
  else {
    // Decode data.
    $data = spdatadrop_decrypt($encoded_data);

    $items['#prefix'] = '<div id="spdatadrop_ajax_form">';
    $items['#suffix'] = '</div>';

    // Check if data is valid.
    if (empty($form_state['data']) && !empty($data['secret'])) {
      $form_state['data'] = $data;
    }

    if (empty($form_state['data'])) {
      $items['feedback'] = array(
        '#markup' => '<p>Uw afmeldlink is helaas niet geldig. Neem contact op met webmaster@sp.nl</p>',
      );
    }
    else {
      $items['choice'] = array(
        '#type' => 'checkbox',
        '#title' => 'Ik wil mijn aanmelding annuleren',
      );

      $items['submit'] = array(
        '#type' => 'submit',
        '#value' => 'Bevestig',
        '#ajax' => array(
          'wrapper' => 'spdatadrop_ajax_form',
          'callback' => 'spdatadrop_ajax_form_callback',
        ),
      );
    }
  }

  return $items;
}

/**
 * Unsubscribe form submit functionality.
 */
function spdatadrop_unsubscribe_form_validate($form, &$form_state) {
  if (empty($form_state['values']['choice'])) {
    form_set_error('choice', 'Zet een vinkje!');
  }
}

/**
 * Unsubscribe form submit functionality.
 */
function spdatadrop_unsubscribe_form_submit($form, &$form_state) {
  $data = $form_state['data'];

  // Store data in sync table.
  $id = db_merge('spdatadrop_unsubscribe')
    ->key(array(
      'secret' => $data['secret'],
    ))
    ->fields(array(
      'date' => time(),
      'secret' => $data['secret'],
      'state' => 0,
    ))
    ->execute();

  if ($id) {
    $form_state['unsubscribed'] = TRUE;
  }
  $form_state['rebuild'] = TRUE;
}

/**
 * Decrypt encrypted string.
 */
function spdatadrop_decrypt($hash) {
  $decoded = base64_decode(urldecode($hash));
  $key = variable_get('spdatadrop_secret_unsubscribe', 'paijeich6zoh4Mah');
  $data = openssl_decrypt($decoded, 'AES-256-CBC', $key, 0, 'Quie9ophei2iKah1');
  $data = unserialize($data);
  return $data;
}

/**
 * SP datadrop unsubscribe form ajax callback.
 */
function spdatadrop_ajax_form_callback($form, &$form_state) {
  return $form;
}

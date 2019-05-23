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

  $items['#prefix'] = '<div id="spdatadrop_ajax_form">';
  $items['#suffix'] = '</div>';

  if (!empty($form_state['unsubscribed'])) {
    // Unsubscription added.
    $items['feedback'] = array(
      '#markup' => '<p>Uw afmelding is opgeslagen. Zodra deze verwerkt is ontvangt u een e-mail ter bevestiging.</p><p>Met vriendelijke groet,<br/>SP</p>',
    );
  }
  else {
    // Decode data.
    $data = spdatadrop_decrypt($encoded_data);

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
      // Check if unsubscribed before, and if so, set defaults.
      $stored_unsubscribe = db_query('SELECT * FROM {spdatadrop_unsubscribe} WHERE secret = :secret', array(':secret' => $data['secret']))->fetchAssoc();

      if (!empty($stored_unsubscribe)) {
        $items['intro'] = array(
          '#markup' => '<p>U heeft zich reeds afgemeld voor: ' . $form_state['data']['title'] . '</p>',
        );
      }
      else {
        $items['intro'] = array(
          '#markup' => '<p>U bent aangemeld op de SP website voor: ' . $form_state['data']['title'] . '</p>',
        );
        $items['unsubscribe'] = array(
          '#type' => 'checkbox',
          '#title' => 'Ik wil deze aanmelding annuleren',
          '#default_value' => TRUE,
        );

        $items['is_opt_out'] = array(
          '#type' => 'checkbox',
          '#title' => 'Ik wil geen e-mails ontvangen van de SP',
        );

        $items['do_not_phone'] = array(
          '#type' => 'checkbox',
          '#title' => 'Ik wil niet gebeld worden door de SP',
        );

        $items['do_not_sms'] = array(
          '#type' => 'checkbox',
          '#title' => 'Ik geen smsjes ontvangen van de SP',
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
  }

  return $items;
}

/**
 * Unsubscribe form validation.
 */
function spdatadrop_unsubscribe_form_validate($form, &$form_state) {
  $values = $form_state['values'];
  if (empty($values['unsubscribe']) && empty($values['do_not_phone']) && empty($values['is_opt_out']) && empty($values['do_not_sms'])) {
    form_set_error('intro', 'U heeft zich nergens voor afgemeld');
  }
}

/**
 * Unsubscribe form submit functionality.
 */
function spdatadrop_unsubscribe_form_submit($form, &$form_state) {
  $data = $form_state['data'];
  $values = $form_state['values'];

  // Store data in sync table.
  $id = db_merge('spdatadrop_unsubscribe')
    ->key(array(
      'secret' => $data['secret'],
    ))
    ->fields(array(
      'date' => time(),
      'unsubscribe' => $values['unsubscribe'],
      'is_opt_out' => $values['is_opt_out'],
      'do_not_phone' => $values['do_not_phone'],
      'do_not_sms' => $values['do_not_sms'],
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

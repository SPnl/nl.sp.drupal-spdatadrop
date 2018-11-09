# nl.sp.drupal-spdatadrop

Drupal module that receives a post request and pushes the data in the request it to CiviCRM contacts.

The admin interface can be found at: admin/config/sp/spdatadrop. On the first tab one can find some statistics about processed datadrops. On the settings page tab a secret needs to be set, and the submission type and quantity need to be chosen. The manual setting exists for testing purposes. The Drupal log contains information about processed datadrops.

Make a post request to submit data. The data structure can be found below. The submission_data and secret entries are obliged. The source_data and debug_info entries are optional.

Example Drupal code:

```
  // Send data to datadrop server.
  $url = 'https://datadrop.sp.nl/spdatadrop';
  $secret = 'mijngeheimesleutel';
  
  $submission_data = array('name' => 'Jannus Joviaal', 'email' => 'joviaal@spnet.nl', groups => array(132, 7834));
  $source_data = array(
    'source_domain' => 'doemee.sp.nl',
    'source_title' => 'Aanmeldformulier v2',
    'source_id' => '98',
    'submission_id' '123',
  );
  $data_to_send = array(
    'submission_data' => $data,
    'source_data' => $source_data,
    'debug_info' => $debug_info,
    'secret' => $secret,
  );

  $request_body = http_build_query($data_to_send);
  $headers = array('Content-Type' => 'application/x-www-form-urlencoded');
  $options = array(
    'method' => 'POST',
    'data' => $request_body,
    'headers' => $headers,
  );
  $result = drupal_http_request($url, $options);
```

**Post data structure:**

* data
    * submission_data
        * contact_id: 746358
        * name: Jannus Joviaal
        * first_name: Jannus
        * middle_name:
        * last_name: Joviaal
        * initials: J.
        * birth_date: 1974-01-28
        * gender: m (m|v|a)
        * email: joviaal@spnet.nl
        * telephone: 0312345678
        * phone_mobile: 0612345678
        * street: Jonkerstraat
        * house_number: 12
        * house_number_addition: a
        * street_and_number: Jonkerstraat 12 a
        * postal_code: 3754 BB
        * locality: Borlichem
        * country: NL
        * notes
            * note_content: Heeft een hond
            * note_use: array(intern, algemeen) 
              opties: intern, telefoongesprek, algemeen
        * overwrite: false (true|false)
        * author: Bregje Bral
        * source: doemee.sp.nl, Aanmeldformulier v2, sid: 123
        * groups: array(132, 7834)
    * source_data
        * source_domain: doemee.sp.nl
        * source_title: Aanmeldformulier v2
        * source_id: 98
        * source_ip: 123:123:123:123
        * submission_id: 123
    * debug_info (array(...))
    * secret: mijnsleutel

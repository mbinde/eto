<?php


function eto_contact_form(&$form_state, $node, $vars) {
  $form['contact_id'] = array (
				   '#type' => 'hidden',
				   '#value' => ($node->contact_id ? $node->contact_id : 0),
				   );
  $form['uid'] = array (
			    '#type' => 'hidden',
			    '#value' => ($node->uid ? $node->uid : $vars['uid']),
			    );
  $form['contact_type'] = array (
				 '#type' => 'hidden',
				 '#value' => ($node->contact_type ? $node->contact_type : $vars['contact_type']),
				 );
  $form['redirect'] = array (
			     '#type' => 'hidden',
			     '#value' => ($node->redirect ? $node->redirect : ($vars['redirect'] ? $vars['redirect'] : 'user')),
			     );
  

  $form['first_name'] = array (
			       '#type' => 'textfield',
			       '#title' => 'First Name',
			       '#default_value' => $node->first_name,
			       '#required' => TRUE,
			       );

  $form['last_name'] = array (
			       '#type' => 'textfield',
			       '#title' => 'Last Name',
			       '#default_value' => $node->last_name,
			       '#required' => TRUE,
			       );

  $form['address_1'] = array (
			       '#type' => 'textfield',
			       '#title' => 'Address Line 1',
			       '#default_value' => $node->address_1,
			       '#required' => TRUE,
			       );
  $form['address_2'] = array (
			       '#type' => 'textfield',
			       '#title' => 'Address Line 2',
			       '#default_value' => $node->address_2,
			       );
  $form['address_3'] = array (
			       '#type' => 'textfield',
			       '#title' => 'Address Line 3',
			       '#default_value' => $node->address_3,
			       );
  $form['city'] = array (
			       '#type' => 'textfield',
			       '#title' => 'City',
			       '#required' => TRUE,
			       '#default_value' => $node->city,
			       );
  $form['state'] = array (
			       '#type' => 'select',
			       '#title' => 'State',
			       '#options' => _get_states('--Select a State--'),
			       '#description' => 'Select "N/A" if you are not in the US',
			       '#required' => TRUE,
			       '#default_value' => ($node->state ? $node->state : 0),
			       );
  $form['zip'] = array (
			       '#type' => 'textfield',
			       '#title' => 'Zip/Postal',
			       '#size' => '10',
			       '#required' => TRUE,
			       '#default_value' => $node->zip,
			       );
  $form['country'] = array (
			       '#type' => 'textfield',
			       '#title' => 'Country',
			       '#size' => '25',
			       '#required' => TRUE,
			       '#default_value' => ($node->country ? $node->country : "United States"),
			       );
  $form['day_phone'] = array (
			       '#type' => 'textfield',
			       '#title' => 'Day Phone',
			       '#size' => '20',
			       '#default_value' => $node->day_phone,
			       );
  $form['evening_phone'] = array (
			       '#type' => 'textfield',
			       '#title' => 'Evening Phone',
			       '#size' => '20',
			       '#required' => TRUE,
			       '#default_value' => $node->evening_phone,
			       );
  $form['cell_phone'] = array (
			       '#type' => 'textfield',
			       '#title' => 'Cell Phone',
			       '#size' => '20',
			       '#default_value' => $node->cell_phone,
			       );
  $form['other_phone'] = array (
			       '#type' => 'textfield',
			       '#title' => 'Other Phone',
			       '#size' => '20',
			       '#default_value' => $node->other_phone,
			       );


  $form['save'] = array (
			   '#type' => 'submit',
			   '#value' => ($vars['save'] ? $vars['save'] : 'Submit'),
			   );

  $form['#submit'] = array('eto_contact_form_submit');

  return $form;
}

function eto_contact_form_submit($form, &$form_state) {

  if ($form_state['values']['contact_id'] > 0) {
    eto_contact_update($form_state['values']);
  } else {
    eto_contact_insert($form_state['values']);
  }

  drupal_set_message("Your contact information has been saved successfully.", "status");

  $form_state['redirect'] = $form_state['values']['redirect'];
}



function eto_contact_load($uid, $type) {
  $result = db_query("SELECT ec.* FROM {eto_contact} AS ec 
                      WHERE ec.contact_id = 
                         (SELECT ecr.contact_id FROM {eto_contact_relationships} AS ecr
                          WHERE ecr.uid = %d AND ecr.contact_type = '%s')",
		     $uid, $type);
  return db_fetch_object($result);
}

function eto_contact_insert($n) {
  global $user;

  $now = time();

  db_query("INSERT INTO {eto_contact}
               (first_name, last_name, address_1, address_2, address_3, city, state, zip, country,
                day_phone, evening_phone, cell_phone, other_phone, created, changed)
            VALUES
               ('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %d, %d)",
	   $n['first_name'], $n['last_name'], $n['address_1'], $n['address_2'], $n['address_3'], $n['city'], $n['state'], $n['zip'], $n['country'],
	   $n['day_phone'], $n['evening_phone'], $n['cell_phone'], $n['other_phone'], $now, $now);

  $id = db_last_insert_id('eto_contact', 'contact_id');

  db_query("INSERT INTO {eto_contact_relationships}
              (uid, contact_id, contact_type)
            VALUES
              (%d, %d, '%s')",
	   $n['uid'], $id, $n['contact_type']);

  watchdog('contacts', 
	   "@user_name inserted a new contact record of type @type",
	   array("@user_name" => $user->name,
		 "@type" => $n['contact_type']),
	   WATCHDOG_NOTICE
	   );
}


function eto_contact_update($n) {
  global $user;

  db_query("UPDATE {eto_contact} SET
               first_name = '%s',
               last_name = '%s',
               address_1 = '%s',
               address_2 = '%s',
               address_3 = '%s',
               city = '%s',
               state = '%s',
               zip = '%s',
               country = '%s',
               day_phone = '%s',
               evening_phone = '%s',
               cell_phone = '%s',
               other_phone = '%s',
               changed = %d
            WHERE
               contact_id = %d",
	   $n['first_name'], $n['last_name'], $n['address_1'], $n['address_2'], $n['address_3'], $n['city'], $n['state'], $n['zip'], $n['country'],
	   $n['day_phone'], $n['evening_phone'], $n['cell_phone'], $n['other_phone'], time(),
	   $n['contact_id']
	   );

  watchdog('profiles', 
	   "@user_name updated a profile",
	   array("@user_name" => $user->name),
	   WATCHDOG_NOTICE,
	   l('view', 'user/' . $n['uid'] . 'tutor/application/consent')
	   );

}



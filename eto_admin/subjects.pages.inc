<?php

function subject_list() {
  $tree = taxonomy_get_tree(2);
  $output .= "<h1>Subjects We Offer</h1>\n";
  $first = 1;
  foreach ($tree as $term) {
    if ($term->depth == 0) {
      if ($first) {
	$first = 0;
      } else {
	$output .= "</ul>\n";
      }
      $output .= "<h2>" . $term->name . "</h2>";
      $output .= "<ul>\n";
    } else {
      $output .= "<li>" . $term->name . "</li>\n";
    }
  }
  $output .= "</ul>\n";
  return $output;
}

function tutor_edit_subject($name = "") {
  global $user;
  if (!$name) {
    $target_user = $user;
  } else {
    $target_user = eto_user_load(array('name' => $name));
  }

  if (!$target_user) {
    drupal_not_found();
    exit;
  }

  if ($target_user->uid != $user->uid) {
    if (! user_acccess('administer the site')) {
      drupal_permission_denied();
      exit;
    }
  }


  $node = sb_tutor_subjects_load($target_user->uid);
  if (!$node) {
    $node = (object) array ('uid' => $target_user->uid);
  }

  $output .= drupal_get_form('tutor_subject_form', $node);
  return $output;
}

function tutor_subject_form (&$form_state, $node) {
  $form = array();
  $tree = taxonomy_get_tree(2);

  // FIXME
  //
  // come back to this, but the idea is creating category_TID categories with checkboxes for the terms in that parent
  // the load function will have to structure a node ... can we just do node->term_TID ?
  //
  // also terms should normally be displayd as tmer (Category) or category - term depending
  //

  $form['uid'] = array ('#type' => 'hidden', '#value' => $node->uid);

  $current_category = "uncategorized-0";
  foreach ($tree as $term) {
    if ($term->depth == 0) {
      // new category
      $current_category = "category-" . $term->tid;
      $form[$current_category] = array (
					'#type' => 'fieldset',
					'#title' => $term->name,
					'#collapsible' => TRUE,
					'#collapsed' => FALSE,
					);
    } else {
      $form[$current_category]["term-" . $term->tid] = array (
							      '#type' => 'checkbox',
							      '#title' => $term->name,
							      '#default_value' => $node->terms->{"term-" . $term->tid},
							      );
    }
  }

  $form['submit'] = array (
			   '#type' => 'submit',
			   '#value' => 'Save My Subjects',
			   );
  $form['#submit'][] = 'tutor_subject_submit_handler';

  return $form;
}

function tutor_subject_submit_handler ($form, &$form_state) {
  foreach ($form_state['values'] as $key => $value) {
    if (preg_match('/^term-(\d+)/', $key, $matches)) {
      $tid = $matches[1];
      if ($value == 1) {
	tutor_subject_insert($form_state['values']['uid'], $tid);
      } else {
	tutor_subject_delete($form_state['values']['uid'], $tid);
      }
    }
  }
  drupal_set_message("Your subjects have been saved successfully.  " . l("Click here to return to your account.", "user"), "success");
}

function tutor_subject_insert($uid, $tid) {
  $c = db_result(db_query("SELECT COUNT(id) FROM {eto_tutor_subjects} WHERE tutor_uid = %d AND tid = %d",
			  $uid, $tid));
  if ($c == 0) {
    db_query("INSERT INTO {eto_tutor_subjects}
                (tutor_uid, tid)
              VALUES (%d, %d)",
	     $uid, $tid);
  }
}

function tutor_subject_delete($uid, $tid) {
  db_query("DELETE FROM {eto_tutor_subjects} WHERE tutor_uid = %d AND tid = %d",
	   $uid, $tid);
}


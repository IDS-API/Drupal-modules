<?php
// $Id;

/**
 * @file
 *   Views query plugin for the IDS API.
 */

class idsviews_views_plugin_query extends views_plugin_query {
  var $request;
  var $orderby;

  /**
   * Constructor; Create the basic query object and fill with default values.
   */
  function init($base_table, $base_field, $options) {
    parent::init($base_table, $base_field, $options);
    module_load_include('inc', 'idsapi', 'idsapi.request');
    $this->request = new IdsApiRequest();
  }

  function add_dataset($dataset) {
    if (isset($dataset)) {
      $this->request->site = $dataset;
    }
  }

  function add_filter($group, $filter, $value) {
    if ($this->check_filter($group, $filter)) {
      $value = preg_replace('/\s*(\&|AND)\s*/i','&',$value); 
      $value = preg_replace('/\s*(\||OR)\s*/i','|',$value); 
      $this->where[$group]['conditions'][] = array(
        'field' => $filter, 
        'value' => $value,
      );
    }
    else {
      watchdog('idsviews', 'Filter !filter cannot be added to more than one group. Filter ignored.', array('!filter' => $filter), WATCHDOG_NOTICE);
    }
  }

  function check_filter($group, $filter) {
    $found = FALSE;
    $groups = array_keys($this->where);
    while (($group_key = current($groups)) && (!$found)) {
      if (($group_key !== $group) && (!empty($this->where[$group_key]['conditions']))) {
        while ((list($field_key, $field_array) = each($this->where[$group_key]['conditions'])) && (!$found)) {
          $found = ($field_array['field'] === $filter);
        }
      }
      next($groups);
    }
    return (!$found);
  }

  function add_field($table, $field, $alias = '', $params = array()) {
    $field_info = array(
      'field' => $field,
      'table' => $table,
      'alias' => $field,
    ) + $params;
    if (empty($this->fields[$field])) {
      $this->fields[$field] = $field_info;
    }
    return $field;
  }

  function add_relationship($left_table, $left_field, $right_table, $right_field) {
      $this->add_field($left_table, $left_field);
      $this->joins[$left_table][$left_field] = array(
        'table' => $right_table,
        'field' => $right_field,
      );
    return FALSE;
  }

  function add_orderby($order, $orderby_field) {
    $this->orderby = array(
      'order' => $order,
      'orderby_field' => $orderby_field,
    );
  }
  
  public function build(&$view) {
    $view->init_pager();
  }

  /**
   * Executes the query and fills the associated view object with according
   * values.
   * @param view $view
   *   The view which is executed.
   */
  function execute(&$view) {
    $type = str_replace("idsviews_", "", $view->base_table);
    $this->request->num_requested = $this->pager->options['items_per_page'];
    $this->request->start_offset = $this->pager->current_page * $this->pager->options['items_per_page'];
    $this->request->object_type = $type;
    $this->request->format = 'full';
    $this->set_url();
    $start = microtime();
    $response = $this->request->makeRequest();
    $view->execute_time = microtime() - $start;
    $view->result = $response->results;
    $this->pager->total_items = $view->total_rows = $response->total_results;
    $this->pager->update_page_info();
  }

  function set_url() {
    if (isset($this->where)) {
      foreach ($this->where as $group) {
        $params = array();
        if (isset($group['type'])) {
          if (strcasecmp($group['type'],"AND") == 0) {
            $oper = '&';
          }
          elseif (strcasecmp($group['type'],"OR") == 0) {
            $oper = '|';
          }
        }
        foreach ($group['conditions'] as $condition) {
          $field = $condition['field'];
          $value = $condition['value'];
          $field = $this->ids_parameter($field);
          $params[$field][] = $value;
        }
        foreach ($params as $param => $values) {
          $value = implode($oper, $values);
          if ($param === 'object_id') {
            $this->request->object_id = $value;
            $this->request->type_request = 'get';
          }
          else {
            $this->request->setSearchParam($param, $value);
          }
        }
      }
    }
    if (isset($this->orderby['order']) && (isset($this->orderby['orderby_field']))) {
      $order = 'sort_' . strtolower($this->orderby['order']);
      $this->request->setSearchParam($order, $this->orderby['orderby_field']);
    }
  }

  function ids_parameter($field) {
    if ($field === 'query_filter') {
      $field = 'q';
    }
    else {
      $field = str_replace('_filter','',$field);
    }
    return $field;
  }

}

<?php
// $Id;

/**
 * @file
 *   IDS API Module default parameters.
 *
 * Sets default values to the API URL, documentation URL, cache time, etc.
 */

define('IDS_API_URL', 'http://api.ids.ac.uk/openapi/');
define('IDS_API_HOME_URL', 'http://api.ids.ac.uk/');
define('IDS_API_KEY_URL', 'http://api.ids.ac.uk/accounts/register/');
define('IDS_API_DOCUMENTATION_URL', 'http://api.ids.ac.uk/docs/');
define('IDS_CACHE', 'idsapi_cache');
define('IDS_DEFAULT_CACHE_TIME', '86400');
define('IDS_DEFAULT_DATASET', 'eldis');
define('IDS_NUM_ITEMS', '30');
define('IDS_API_KEY_PAR', '_token_guid');
define('IDS_DEFAULT_FORMAT', 'full');
define('IDS_DEFAULT_TYPE_REQUEST', 'search');
define('IDS_CSS_OBJECT', 'ids_object');
define('IDS_PREFIX', 'IDS');
define('IDS_BASE_CATEGORY_ID', '');
// IDS_MAX_NUM_ITEMS is the max. size of the page allowed by the API. Please read the API documentation before changing it: http://api.ids.ac.uk/docs/
define('IDS_MAX_NUM_ITEMS', '500');








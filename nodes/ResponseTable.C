/*
 * This table handles multiple responses, acting only on the last one expected.
 * To use this structure, you must provide a unique identifier for responses
 * and the exact number of entries that will arrive with that ID. This will be
 * used to call a callback function when the ID has appeared in the specified
 * number of responses.
 *
 * For example, a thread my communicate with 10 other threads, each of which
 * will respond asynchronously. This table will hold the first 9 responses,
 * then make a callback when the tenth arrives, passing the value 10 followed
 * by an array of the responses.
 */

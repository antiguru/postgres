/*
 * sprocket.c
 *
 *  Created on: Feb 28, 2014
 *      Author: moritz
 */

#include <string.h>

#include "postgres.h"

#include "sprocket.h"

#include "fmgr.h"
#include "access/transam.h"
#include "access/xact.h"
#include "executor/spi.h"
#include "commands/trigger.h"
#include "commands/event_trigger.h"

#include "utils/json.h"

PG_MODULE_MAGIC
;

Datum
sprocket_change(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(sprocket_change);

Datum
sprocket_ddl(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(sprocket_ddl);

void _PG_init(void);
void _PG_fini(void);

void SprocketXactEvent(XactEvent event, void *arg);

void SprocketSubXactEvent(SubXactEvent event, SubTransactionId mySubid,
		SubTransactionId parentSubid, void *arg);

extern void RegisterXactCallback(XactCallback callback, void *arg);
extern void UnregisterXactCallback(XactCallback callback, void *arg);
extern void RegisterSubXactCallback(SubXactCallback callback, void *arg);
extern void UnregisterSubXactCallback(SubXactCallback callback, void *arg);

static TransactionId transaction = InvalidTransactionId;
static int count = 0;

void _PG_init() {
	printf("%s test\n", __func__);
	RegisterXactCallback(&SprocketXactEvent, NULL);
	RegisterSubXactCallback(&SprocketSubXactEvent, NULL);
}

void _PG_fini() {
	printf("%s\n", __func__);
	UnregisterXactCallback(&SprocketXactEvent, NULL);
	UnregisterSubXactCallback(&SprocketSubXactEvent, NULL);
}

void SprocketXactEvent(XactEvent event, void *arg) {
	printf("event %i, txn %i, count %i\n", event, GetCurrentTransactionId(),
			count);
}

void SprocketSubXactEvent(SubXactEvent event, SubTransactionId mySubid,
		SubTransactionId parentSubid, void *arg) {
	printf("subevent %i\n", event);
}

Datum sprocket_change(PG_FUNCTION_ARGS) {
	TriggerData *trigdata = (TriggerData *) fcinfo->context;
	TupleDesc tupdesc;
	HeapTuple rettuple;
	char *when;
	bool checknull = false;
	bool isnull = false;
//	int ret;
	int i;
	if (!CALLED_AS_TRIGGER(fcinfo)) {
		elog(ERROR, "must be called as a trigger");
	}
	if (!IsTransactionOrTransactionBlock()) {
		elog(ERROR, "can only record changes in transaction");
	}
	for (i = 0; i < fcinfo->nargs; i++) {
		printf(" %i: %p", i, (void *) fcinfo->arg[i]);
	}
	/* tuple to return to executor */
	if (TRIGGER_FIRED_BY_UPDATE(trigdata->tg_event))
		rettuple = trigdata->tg_newtuple;
	else
		rettuple = trigdata->tg_trigtuple;

	/* check for null values */
	if (!TRIGGER_FIRED_BY_DELETE(
			trigdata->tg_event) && TRIGGER_FIRED_BEFORE(trigdata->tg_event))
		checknull = true;

	if (TRIGGER_FIRED_BEFORE(trigdata->tg_event))
		when = "before";
	else
		when = "after ";

	tupdesc = trigdata->tg_relation->rd_att;

	printf("%s tupdesc.natts %i %i %i\n", when, tupdesc->natts, checknull,
			isnull);

	for (i = 0; i < tupdesc->natts; i++) {
		printf("%s %s %s\n", SPI_fname(tupdesc, i + 1),
				SPI_gettype(tupdesc, i + 1),
				SPI_getvalue(rettuple, tupdesc, i + 1));

	}

	if (GetCurrentTransactionId() != transaction) {
		transaction = GetCurrentTransactionId();
		count = 1;
	} else {
		count++;
	}

	return PointerGetDatum(rettuple);
}

Datum sprocket_ddl(PG_FUNCTION_ARGS) {
	EventTriggerData *trigdata;

	if (!CALLED_AS_EVENT_TRIGGER(fcinfo)) /* internal error */
		elog(ERROR, "not fired by event trigger manager");

	trigdata = (EventTriggerData *) fcinfo->context;

	ereport(ERROR,
			(errcode(ERRCODE_INSUFFICIENT_PRIVILEGE), errmsg("command \"%s\" denied", trigdata->tag)));

	PG_RETURN_NULL()
	;
}



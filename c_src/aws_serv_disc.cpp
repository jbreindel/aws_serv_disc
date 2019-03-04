#include <aws/core/Aws.h>
#include <aws/core/utils/Outcome.h>
#include <aws/servicediscovery/ServiceDiscoveryClient.h>
#include <aws/servicediscovery/model/ListInstancesRequest.h>
#include <erl_nif.h>
#include <string.h>

using namespace Aws::Client;
using namespace Aws::ServiceDiscovery;
using namespace Aws::ServiceDiscovery::Model;

ERL_NIF_TERM mk_atom(ErlNifEnv* env, const char* atom) {
	ERL_NIF_TERM ret;
	if(!enif_make_existing_atom(
		env, atom, &ret, ERL_NIF_LATIN1)) {
		return enif_make_atom(env, atom);
	}
	return ret;
}

ERL_NIF_TERM mk_error(ErlNifEnv* env, const char* mesg) {
	return enif_make_tuple2(
		env, mk_atom(env, "error"), mk_atom(env, mesg));
}

static ERL_NIF_TERM list_instances(
	ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
	if (argc != 1) {
		return enif_make_badarg(env);
	}
	char serv_id[1024];
	memset(&serv_id, '\0', sizeof(serv_id));
	if (enif_get_string(env, argv[0],
		serv_id, sizeof(serv_id), ERL_NIF_LATIN1) < 1) {
		return enif_make_badarg(env);
	}
	ERL_NIF_TERM res = mk_error(env, "Can't Fetch");
	Aws::SDKOptions opts;
	Aws::InitAPI(opts);
	{
		const Aws::String servId(serv_id);
		ClientConfiguration clientConfig;
		ServiceDiscoveryClient servDiscClient(clientConfig);
		ListInstancesRequest req;
		req.SetServiceId(servId);
		auto outcome = servDiscClient.ListInstances(req);
		if (outcome.IsSuccess()) {
			Aws::Vector<InstanceSummary> summaries =
				outcome.GetResult().GetInstances();
			ERL_NIF_TERM arr[summaries.size()];
			memset(&arr, 0, sizeof(arr));
			int err = 0;
			int i = 0;
			for (auto const &sum : summaries) {
				ERL_NIF_TERM id = enif_make_string(
					env, sum.GetId().c_str(), ERL_NIF_LATIN1);
				size_t attr_sz = sum.GetAttributes().size();
				ERL_NIF_TERM attr_keys[attr_sz];
				ERL_NIF_TERM attr_vals[attr_sz];
				int a = 0;
				for (auto const &kv : sum.GetAttributes()) {
					attr_keys[a] = enif_make_string(
						env, kv.first.c_str(), ERL_NIF_LATIN1);
					attr_vals[a] = enif_make_string(
						env, kv.second.c_str(), ERL_NIF_LATIN1);
					a++;
				}
				ERL_NIF_TERM attrs;
				if (!enif_make_map_from_arrays(
					env, attr_keys, attr_vals, attr_sz, &attrs)) {
					err = 1;
					break;
				}
				ERL_NIF_TERM inst;
				ERL_NIF_TERM inst_keys[1];
				ERL_NIF_TERM inst_vals[1];
				inst_keys[0] = id;
				inst_vals[0] = attrs;
				if (!enif_make_map_from_arrays(
					env, inst_keys, inst_vals, 1, &inst)) {
					err = 1;
					break;
				}
				arr[i] = inst;
				i++;
			}
			if (!err) {
				res = enif_make_list_from_array(
					env, arr, summaries.size());
			}
		}
	}
	Aws::ShutdownAPI(opts);
	return res;
}

static ErlNifFunc nif_funcs[] = {
	{"list_instances", 1,
		list_instances, ERL_NIF_DIRTY_JOB_IO_BOUND}
};

ERL_NIF_INIT(aws_serv_disc,
	nif_funcs, NULL, NULL, NULL, NULL);

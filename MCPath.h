
// supporting tools for generic Monte Carlo

namespace MonteCarlo
{
	// memory used by the stepper
	struct Workspace_ : noncopyable
	{
		virtual ~Workspace_();
		virtual Workspace_* Clone() const = 0;
	};
	// information shared through time (viewed by multiple steppers)
	struct PathsRecord_ : noncopyable
	{
		virtual ~PathsRecord_();
		virtual void StartPath(int i_path) = 0;
	};
	typedef std::shared_ptr<PathsRecord_> record_t;
}

package tasks

import (
	"testing"

	"evals.mitre.org/control_server/util/test_util/assert_util"
)

const (
	testGUID = "123456789"
	testCommand = "pwd"
	mockUuid = "00000000-0000-0000-0000-000000000000"
)

var expectedTask = Task{
	GUID:		testGUID,
	Command:	testCommand,
	Output:		"",
	Status:		TASK_STATUS_NEW,
	ExitCode:	-1,
}

var expectedTaskMockUuid = Task{
	GUID:		mockUuid,
	Command:	testCommand,
	Output:		"",
	Status:		TASK_STATUS_NEW,
	ExitCode:	-1,
}

func mockGenerateUUID() string {
	return mockUuid
}

// TaskFactory
func TestTaskFactoryGUIDProvided(t *testing.T) {
	TaskFactory(testCommand, testGUID, GenerateUUID)
	newTask, valueExists := GetTaskById(testGUID)
	if valueExists != nil {
		t.Fatalf("Expected new Task with guid %v, but no Task found\n", testGUID)
	}
	assert_util.AssertDeepEq(t, expectedTask, *newTask, "")
}

// TaskFactory
func TestTaskFactoryNoGUID(t *testing.T) {
	newTask := TaskFactory(testCommand, "", mockGenerateUUID)
	_, valueExists := GetTaskById(newTask.GUID)
	if valueExists != nil {
		t.Fatalf("Expected new Task to be created, but no Task found\n")
	}

	// Test expected default values
	assert_util.AssertDeepEq(t, expectedTaskMockUuid, *newTask, "")
}

// GetTaskById
func TestGetNonExistentTaskById(t *testing.T) {
	invalidGUID := "000000000"
	_, result := GetTaskById(invalidGUID)

	if result == nil {
		t.Fatalf("Expected error message for missing Task %v, but found Task\n", invalidGUID)
	}
}

// GetTaskOutputById
func TestGetTaskOutputById(t *testing.T) {
	guid := "get-task-output-guid"
	expectedOutput := "username"
	newTask := TaskFactory(testCommand, guid, mockGenerateUUID)
	(*newTask).Output = expectedOutput

	// Test expected default values
	result, err := GetTaskOutputById((*newTask).GUID)
	if err != nil {
		t.Fatalf("Task %s not found: %s", (*newTask).GUID, err.Error())
	}
	assert_util.AssertEq(t, expectedOutput, result, "")
}

// SetTaskOutputById
func TestSetTaskOutputById(t *testing.T) {
	guid := "set-task-output-guid"
	newTask := TaskFactory(testCommand, guid, mockGenerateUUID)
	setOutput := "command output"

	assert_util.AssertEq(t, newTask.Status, TASK_STATUS_NEW, "")

	SetTaskOutputById(guid, setOutput, true)

	// Test expected default values
	assert_util.AssertEq(t, setOutput, (*newTask).Output, "")

	// Make sure task finished
	assert_util.AssertEq(t, newTask.Status, TASK_STATUS_FINISHED, "")

	guid2 := "set-task-output-not-complete"
	newTask2 := TaskFactory(testCommand, guid, mockGenerateUUID)
	assert_util.AssertEq(t, newTask2.Status, TASK_STATUS_NEW, "")
	SetTaskOutputById(guid2, setOutput, false)
	assert_util.AssertEq(t, newTask2.Status, TASK_STATUS_NEW, "")
}

// CancelTask
func TestCancelTask(t *testing.T) {
	var testTask = Task{
		GUID:		"1234-test-uuid",
		Command:	testCommand,
		Output:		"",
		Status:		TASK_STATUS_NEW,
		ExitCode:	-1,
	}
	testTask.CancelTask()
    assert_util.AssertEq(t, testTask.Status, TASK_STATUS_DISCARDED, "")
}

// GenerateUUID
func TestGenerateUUID(t *testing.T) {
	guid := GenerateUUID()

	if len(guid) != 36 {
		t.Fatalf("Expected randomly generated guid, got empty string\n")
	}
}

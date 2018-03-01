/**********************************************************************\
 * ext4-browser
 * backendTask.h
 *
 * backend task interface
 **********************************************************************
 * Copyright (C) 2017 - Max Maisel
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
\**********************************************************************/
#pragma once

class IBackendTask
{
	public:
		IBackendTask()
			: mCanceled(false)
			, mQuestionResult(0) {}
		virtual ~IBackendTask() {}

		enum
		{
			ANSWER_NOP,
			QUESTION_FILE_EXISTS = wxID_HIGHEST+1,
			QUESTION_UMOUNT,
			QUESTION_ERROR
		};

		virtual void OnProgress(int n) = 0;
		virtual int  OnFileExists(const wxString& filename, bool isFolder) = 0;
		virtual int  OnError(const wxString& filename) = 0;

		inline void Cancel() {mCanceled.store(true);}
		inline void ResetCanceled() {mCanceled.store(false);}
		inline bool IsCanceled() const {return mCanceled.load();}

		inline int WaitForAnswer()
		{
			mQuestionResult = ANSWER_NOP;
			do
			{
				std::unique_lock<std::mutex> lk(mQuestionMutex);
				mQuestion.wait(lk);
			}
			// Spurious wakeup
			while(mQuestionResult == ANSWER_NOP);

			return mQuestionResult;
		}

		inline void AnswerQuestion(int answer)
		{
			std::unique_lock<std::mutex> lk(mQuestionMutex);
			mQuestionResult = answer;
			lk.unlock();
			mQuestion.notify_one();
		}

	protected:
		std::atomic<bool> mCanceled;
		std::mutex mQuestionMutex;
		std::condition_variable mQuestion;
		int mQuestionResult;
};


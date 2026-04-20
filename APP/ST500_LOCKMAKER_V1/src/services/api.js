/**
 * ST500 LockMaker API 서비스
 * 서버: http://115.68.227.31/api/st500/st500_api.php
 * TODO: 운영 배포 시 HTTPS 서버로 변경 필수
 */

const BASE_URL = import.meta.env.VITE_API_BASE_URL
  ?? 'http://115.68.227.31/api/st500/st500_api.php'

/**
 * @param {string} params - URL 쿼리스트링 (예: "code=get_device&device_id=xxx")
 * @returns {Promise<any[]>} 파싱된 JSON 배열
 */
async function request(params) {
  const url = `${BASE_URL}?${params}`
  const res = await fetch(url, {
    method: 'GET',
    headers: { 'Content-Type': 'application/json' }
  })
  if (!res.ok) throw new Error(`HTTP ${res.status}`)
  const text = await res.text()
  if (!text || text.length < 20) throw new Error('응답 데이터 부족')
  return JSON.parse(text)
}

/**
 * 디바이스 등록 (최초 실행 또는 이름 변경 시)
 * @param {string} deviceId
 * @param {string} name
 * @returns {Promise<{msg: string}>}
 */
export async function registerDevice(deviceId, name) {
  const data = await request(
    `code=send_device&device_id=${encodeURIComponent(deviceId)}&name=${encodeURIComponent(name)}`
  )
  return data[0]
}

/**
 * 디바이스 승인 상태 조회
 * @param {string} deviceId
 * @returns {Promise<{msg: string}>}  msg === 'approve' 이면 승인 완료
 */
export async function getDeviceStatus(deviceId) {
  const data = await request(
    `code=get_device&device_id=${encodeURIComponent(deviceId)}`
  )
  return data[0]
}
